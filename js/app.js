var layerTree = function (options) {
    'use strict';
    if (!(this instanceof layerTree)) {
        throw new Error('layerTree must be constructed with the new keyword.');
    } else if (typeof options === 'object' && options.map && options.target) {
        if (!(options.map instanceof ol.Map)) {
            throw new Error('Please provide a valid OpenLayers 3 map object.');
        }
        this.map = options.map;
        var containerDiv = document.getElementById(options.target);
        if (containerDiv === null || containerDiv.nodeType !== 1) {
            throw new Error('Please provide a valid element id.');
        }
        this.messages = document.getElementById(options.messages) || document.createElement('span');
        var controlDiv = document.createElement('div');
        controlDiv.className = 'layertree-buttons';
		
		
        // 增加本地图层按钮
        controlDiv.appendChild(this.createButton('addvector', '本地图层', 'addlayer'));
        controlDiv.appendChild(this.createButton('newvector', '新建标注层', 'addlayer'));

		// 删除图层按钮
        controlDiv.appendChild(this.createButton('deletelayer', '删除图层', 'deletelayer'));
        
        //controlDiv.appendChild(this.createButton('itemprops', '图元属性', 'itemprops'));

        containerDiv.appendChild(controlDiv);
        this.layerContainer = document.createElement('div');
        this.layerContainer.className = 'layercontainer';
        containerDiv.appendChild(this.layerContainer);
        var idCounter = 0;
        this.selectedLayer = null;//当前选中层
        // 可收听对象
        this.selectEventEmitter = new ol.Observable();

        this.createRegistry = function (layer, buffer) {
            layer.set('id', 'layer_' + idCounter);
            idCounter += 1;
            var layerDiv = document.createElement('div');
            layerDiv.className = buffer ? 'layer ol-unselectable buffering' : 'layer ol-unselectable';
            layerDiv.title = layer.get('name') || 'Unnamed Layer';
            layerDiv.id = layer.get('id');
			// 增加单击事件
			this.addSelectEvent(layerDiv);
			
            var layerSpan = document.createElement('span');
            layerSpan.textContent = layerDiv.title;
            layerDiv.appendChild(this.addSelectEvent(layerSpan, true));
            this.layerContainer.insertBefore(layerDiv, this.layerContainer.firstChild);
            return this;
        };
		
		// 图层添加事件
        this.map.getLayers().on('add', function (evt) {
            if (evt.element instanceof ol.layer.Vector) {
                this.createRegistry(evt.element, true);
            } else {
                this.createRegistry(evt.element);
            }
        }, this);
		
		// 图层删除事件
		this.map.getLayers().on('remove', function (evt) {
            this.removeRegistry(evt.element);

            this.selectEventEmitter.changed();
        }, this);
		
    } else {
        throw new Error('Invalid parameter(s) provided.');
    }
};

/**
 * 图层控制器上的功能按钮
 * @param {按钮id} elemName 
 * @param {按钮标题} elemTitle 
 * @param {按钮的类型，实现中用于路由不同的处理方法} elemType 
 */
layerTree.prototype.createButton = function (elemName, elemTitle, elemType) {
    var buttonElem = document.createElement('button');
    buttonElem.className = elemName;
    buttonElem.title = elemTitle;
    switch (elemType) {
        case 'addlayer':
            //增加本地矢量文件图层
            buttonElem.addEventListener('click', function () {
                document.getElementById(elemName).style.display = 'block';
            });
            return buttonElem;
        case 'deletelayer':
            //删除选中的图层
            var _this = this;
            buttonElem.addEventListener('click', function () {
                if (_this.selectedLayer) {
                    var layer = _this.getLayerById(_this.selectedLayer.id);
                    _this.map.removeLayer(layer);
                    _this.messages.textContent = 'Layer removed successfully.';
                } else {
                    _this.messages.textContent = 'No selected layer to remove.';
                }
            });
            return buttonElem;
        // case 'itemprops':
        //     var _this = this;
        //     buttonElem.addEventListener('click', function () {
        //         document.getElementById(elemName).style.display = 'block';
        //     });
        //     return buttonElem;			
        default:
            return false;
    }
};

layerTree.prototype.addBufferIcon = function (layer) {
    layer.getSource().on('change', function (evt) {
        var layerElem = document.getElementById(layer.get('id'));
        switch (evt.target.getState()) {
            case 'ready':
                layerElem.className = layerElem.className.replace(/(?:^|\s)(error|buffering)(?!\S)/g, '');
                break;
            case 'error':
                layerElem.classList.add('error');
                break;
            default:
                layerElem.classList.add('buffering');
                break;
        }
    });
};

layerTree.prototype.removeContent = function (element) {
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }
    return this;
};

layerTree.prototype.createOption = function (optionValue) {
    var option = document.createElement('option');
    option.value = optionValue;
    option.textContent = optionValue;
    return option;
};


layerTree.prototype.addVectorLayer = function (form) {
    var file = form.file.files[0];
    var currentProj = this.map.getView().getProjection();
    var fr = new FileReader();
    var sourceFormat;
    var source = new ol.source.Vector();
    fr.onload = function (evt) {
        //文件读取到的内容
        var vectorData = evt.target.result;
        switch (form.format.value) {
            case 'geojson':
                sourceFormat = new ol.format.GeoJSON();
                break;
            case 'topojson':
                sourceFormat = new ol.format.TopoJSON();
                break;
            case 'kml':
                sourceFormat = new ol.format.KML();
                break;
            case 'osm':
                sourceFormat = new ol.format.OSMXML();
                break;
            default:
                return false;
        }
        /**按照如下次序获取空间参考：用户界面选定->数据中自描述->当前显示 */
        var dataProjection = form.projection.value || sourceFormat.readProjection(vectorData) || currentProj;
        source.addFeatures(sourceFormat.readFeatures(vectorData, {
            dataProjection: dataProjection,
            featureProjection: currentProj
        }));
    };
    fr.readAsText(file);
    var layer = new ol.layer.Vector({
        source: source,
        name: form.displayname.value
    });
    this.addBufferIcon(layer);
    this.map.addLayer(layer);
    this.messages.textContent = 'Vector layer added successfully.';
    return this;
};

layerTree.prototype.newVectorLayer = function (form) {
    var type = form.type.value;
    if (type !== 'point' && type !== 'line' && type !== 'polygon' && type !== 'geomcollection') {
        this.messages.textContent = 'Unrecognized layer type.';
        return false;
    }
    var layer = new ol.layer.Vector({
        source: new ol.source.Vector(),
        name: form.displayname.value || '未命名图层',
        type: type
    });
    this.addBufferIcon(layer);
    this.map.addLayer(layer);
    layer.getSource().changed();
    this.messages.textContent = 'New vector layer created successfully.';
    return this;
};


/**选中当前的图层 */
layerTree.prototype.addSelectEvent = function (node, isChild) {
    var _this = this;
    node.addEventListener('click', function (evt) {
        var targetNode = evt.target;
        if (isChild) {
            evt.stopPropagation();
            targetNode = targetNode.parentNode;
        }
        if (_this.selectedLayer) {
            _this.selectedLayer.classList.remove('active');
        }
        _this.selectedLayer = targetNode;
        targetNode.classList.add('active');
        _this.selectEventEmitter.changed();
    });
    return node;
};

/**删除当前图层的节点 */
layerTree.prototype.removeRegistry = function (layer) {
    var layerDiv = document.getElementById(layer.get('id'));
    this.layerContainer.removeChild(layerDiv);
    return this;
};
/**通过图层id获取图层对象 */
layerTree.prototype.getLayerById = function (id) {
    var layers = this.map.getLayers().getArray();
    for (var i = 0; i < layers.length; i += 1) {
        if (layers[i].get('id') === id) {
            return layers[i];
        }
    }
    return false;
};

/**工具条 */
var toolBar = function (options) {
    'use strict';
    if (!(this instanceof toolBar)) {
        throw new Error('toolBar must be constructed with the new keyword.');
    } else if (typeof options === 'object' && options.map && options.target && options.layertree) {
        if (!(options.map instanceof ol.Map)) {
            throw new Error('Please provide a valid OpenLayers 3 map object.');
        }
        this.map = options.map;
        this.toolbar = document.getElementById(options.target);
        this.layertree = options.layertree;
        this.controls = new ol.Collection();
    } else {
        throw new Error('Invalid parameter(s) provided.');
    }
};

toolBar.prototype.addControl = function (control) {
    if (!(control instanceof ol.control.Control)) {
        throw new Error('Only controls can be added to the toolbar.');
    }
    if (control.get('type') === 'toggle') {
        control.on('change:active', function () {
            if (control.get('active')) {
                this.controls.forEach(function (controlToDisable) {
                    if (controlToDisable.get('type') === 'toggle' && controlToDisable !== control) {
                        controlToDisable.set('active', false);
                    }
                });
            }
        }, this);
    }
    control.setTarget(this.toolbar);
    this.controls.push(control);
    this.map.addControl(control);
    return this;
};

toolBar.prototype.removeControl = function (control) {
    this.controls.remove(control);
    this.map.removeControl(control);
    return this;
};

////////////////////////




ol.control.Cesium = function (opt_options) {
    var options = opt_options || {};
    var _this = this;
    var controlDiv = document.createElement('div');
    controlDiv.className = options.class || 'ol-cesium ol-unselectable ol-control';
    setTimeout(function () {
        var ol3d = new olcs.OLCesium({map: _this.getMap()});
        var scene = ol3d.getCesiumScene();
		
		//场景中打开地形
        //scene.terrainProvider = Cesium.createWorldTerrain();

        _this.set('cesium', ol3d);
    }, 2000);
	
	// 向工具条上增加2-3D切换按钮
    var controlButton = document.createElement('button');
    controlButton.textContent = '3D';
    controlButton.title = 'Toggle 3D rendering';
    controlButton.addEventListener('click', function (evt) {
        var cesium = _this.get('cesium');
        if (cesium.getEnabled()) {
            cesium.setBlockCesiumRendering(true);
            cesium.setEnabled(false);
        } else {
            cesium.setBlockCesiumRendering(false);
            cesium.setEnabled(true);
        }
    });
    controlDiv.appendChild(controlButton);
    ol.control.Control.call(this, {
        element: controlDiv,
        target: options.target
    });
};
ol.inherits(ol.control.Cesium, ol.control.Control);

////////////////////////////
//二维窗口中的目标选取
ol.control.Interaction = function (opt_options) {
    var options = opt_options || {};

    // 创建按钮的容器层，设置css样式并在div中创建一个按钮
    var controlDiv = document.createElement('div');
    controlDiv.className = options.className || 'ol-unselectable ol-control';
    var controlButton = document.createElement('button');
    controlButton.textContent = options.label || 'S';
    controlButton.title = options.tipLabel || '目标选择';
    controlDiv.appendChild(controlButton);

    this.setDisabled = function (bool) {
        if (typeof bool === 'boolean') {
            controlButton.disabled = bool;
            return this;
        }
    };

    var _this = this;
    //设置交互工具的状态
    controlButton.addEventListener('click', function () {
        if (_this.get('interaction').getActive()) {
            _this.set('active', false);
        } else {
            _this.set('active', true);
        }
    });

    var interaction = options.interaction;
    ol.control.Control.call(this, {
        element: controlDiv,
        target: options.target
    });
    // 设置交互的属性集合，继承自ol.Object
    this.setProperties({
        interaction: interaction,
        active: false,
        type: 'toggle',
        destroyFunction: function (evt) {
            if (evt.element === _this) {
                this.removeInteraction(_this.get('interaction'));
            }
        }
    });

    // ol.Object::on 当事件发生时的响应
    this.on('change:active', function () {
        this.get('interaction').setActive(this.get('active'));
        // 根据交互工具的状态，设置按钮的css样式(添加或移除 'active')
        if (this.get('active')) {
            controlButton.classList.add('active');
        } else {
            controlButton.classList.remove('active');
        }
    }, this);
};
ol.inherits(ol.control.Interaction, ol.control.Control);

ol.control.Interaction.prototype.setMap = function (map) {
    ol.control.Control.prototype.setMap.call(this, map);
    var interaction = this.get('interaction');
    if (map === null) {
        // 如果设置的图为空，去除由on或once返回的ol.Observable 监听
        ol.Observable.unByKey(this.get('eventId'));
    } else if (map.getInteractions().getArray().indexOf(interaction) === -1) {
        // 如果地图对象不为空，则检查本交互是否在地图中，如果不在地图中则将其加入到地图中
        map.addInteraction(interaction);
        interaction.setActive(false);
        this.set('eventId', map.getControls().on('remove', this.get('destroyFunction'), map));
    }
};



toolBar.prototype.addSelectControls = function(){
    var layertree = this.layertree;
    //仅选择图层控制器中的当前层
    var selectInteraction = new ol.interaction.Select({
        layers: function (layer) {
            if (layertree.selectedLayer) {
                if (layer === layertree.getLayerById(layertree.selectedLayer.id)) {
                    return true;
                }
            }
            return false;
        }
    });

    var selected_feature = null;
    selectInteraction.on('select',function(e){
        if(e.selected.length>0)
        {
            selected_feature = e.selected[0];
            var itempropsDiv = document.getElementById('itemprops');
            itempropsDiv.style.display = 'block';
            ////////////////////////////////////////////
            function updateFeatureStyle(evt){
                // 阻止默认的提交行为
                evt.preventDefault();
                // 隐藏显示的内容
                this.parentNode.style.display = 'none';
                
                /**
                 * 从用户的界面输入获取相应的属性
                 */
                
               
                //将从form中获取的RGB HEX转换成为ol [r,g,b,a]数组
                var fillColor = [parseInt(this.fillColor.value.substring(1,3),16),
                                    parseInt(this.fillColor.value.substring(3,5),16),
                                    parseInt(this.fillColor.value.substring(5,7),16),0.5];

                var strokeColor = [parseInt(this.strokeColor.value.substring(1,3),16),
                    parseInt(this.strokeColor.value.substring(3,5),16),
                    parseInt(this.strokeColor.value.substring(5,7),16),1];

               
                
                var updatedStyle = new ol.style.Style({
                    image: new ol.style.RegularShape({
                        stroke: new ol.style.Stroke({
                            width: parseInt(this.strokeSize.value/4),
                            color: strokeColor
                        }),

                        fill: new ol.style.Fill({
                            color: fillColor
                        }),
                        points: 5,
                        
                        radius1: parseInt(this.strokeSize.value),
                        radius2: parseInt(this.strokeSize.value*3/2),
                        
                   
                        rotation: Math.PI
                    })});
            
                if(selected_feature !=null)
                {
                    selected_feature.setStyle(updatedStyle);
                }
                
                document.getElementById('itemprops_form').removeEventListener(updateFeatureStyle);
            }

            document.getElementById('itemprops_form').addEventListener('submit',updateFeatureStyle);

            //////////////////////////////////////////
        }else
        {
            alert('no item selected!');
            selected_feature = null;
        }
        
    });

    var selectSingle = new ol.control.Interaction({
        label: ' ',
        tipLabel: '目标单选',
        className: 'ol-singleselect ol-unselectable ol-control',
        interaction: selectInteraction
    });

    // 取消选取
    var controlDiv = document.createElement('div');
    controlDiv.className = 'ol-deselect ol-unselectable ol-control';
    var controlButton = document.createElement('button');
    controlButton.title = 'Remove selection(s)';
    controlDiv.appendChild(controlButton);
    controlButton.addEventListener('click', function () {
        selectInteraction.getFeatures().clear();
    });
    var deselectControl = new ol.control.Control({
        element: controlDiv
    });
    this.addControl(selectSingle)
        .addControl(deselectControl);
    return this;


};

toolBar.prototype.addEditingToolBar = function () {
    var layertree = this.layertree;
    this.editingControls = new ol.Collection();
    var drawPoint = new ol.control.Interaction({
        label: ' ',
        tipLabel: 'Add points',
        className: 'ol-addpoint ol-unselectable ol-control',
        interaction: this.handleEvents(new ol.interaction.Draw({
            type: 'Point',
            snapTolerance: 1
        }), 'point')
    }).setDisabled(true);
    this.editingControls.push(drawPoint);
    var drawLine = new ol.control.Interaction({
        label: ' ',
        tipLabel: 'Add lines',
        className: 'ol-addline ol-unselectable ol-control',
        interaction: this.handleEvents(new ol.interaction.Draw({
            type: 'LineString',
            snapTolerance: 1
        }), 'line')
    }).setDisabled(true);
    this.editingControls.push(drawLine);
    var drawPolygon = new ol.control.Interaction({
        label: ' ',
        tipLabel: 'Add polygons',
        className: 'ol-addpolygon ol-unselectable ol-control',
        interaction: this.handleEvents(new ol.interaction.Draw({
            type: 'Polygon',
            snapTolerance: 1
        }), 'polygon')
    }).setDisabled(true);
    this.editingControls.push(drawPolygon);
    layertree.selectEventEmitter.on('change', function () {
        var layer = layertree.getLayerById(layertree.selectedLayer.id);
        if (layer instanceof ol.layer.Vector) {
            this.editingControls.forEach(function (control) {
                control.setDisabled(false);
            });
            var layerType = layer.get('type');
            if (layerType !== 'point' && layerType !== 'geomcollection') drawPoint.setDisabled(true).set('active', false);
            if (layerType !== 'line' && layerType !== 'geomcollection') drawLine.setDisabled(true).set('active', false);
            if (layerType !== 'polygon' && layerType !== 'geomcollection') drawPolygon.setDisabled(true).set('active', false);
        } else {
            this.editingControls.forEach(function (control) {
                control.set('active', false);
                control.setDisabled(true);
            });
        }
    }, this);
    this.addControl(drawPoint).addControl(drawLine).addControl(drawPolygon);
    return this;
};

toolBar.prototype.handleEvents = function (interaction, type) {
    if (type !== 'point') {
        interaction.on('drawstart', function (evt) {
            var error = false;
            if (this.layertree.selectedLayer) {
                var selectedLayer = this.layertree.getLayerById(this.layertree.selectedLayer.id);
                var layerType = selectedLayer.get('type');
                error = (layerType !== type && layerType !== 'geomcollection') ? true : false;
            } else {
                error = true;
            }
            if (error) {
                interaction.finishDrawing();
            }
        }, this);
    }
    interaction.on('drawend', function (evt) {
        var error = '';
        errorcheck: if (this.layertree.selectedLayer) {
            var selectedLayer = this.layertree.getLayerById(this.layertree.selectedLayer.id);
            error = selectedLayer instanceof ol.layer.Vector ? '' : 'Please select a valid vector layer.';
            if (error) break errorcheck;
            var layerType = selectedLayer.get('type');
            error = (layerType === type || layerType === 'geomcollection') ? '' : 'Selected layer has a different vector type.';
        } else {
            error = 'Please select a layer first.';
        }
        if (! error) {
            selectedLayer.getSource().addFeature(evt.feature);
        } else {
            this.layertree.messages.textContent = error;
        }
    }, this);
    return interaction;
};


///////////////////////////////


function init() {
    document.removeEventListener('DOMContentLoaded', init);
	
	//增加拖放交互
	var dragAndDrop = new ol.interaction.DragAndDrop({
        formatConstructors: [
            ol.format.GeoJSON,
            ol.format.TopoJSON,
            ol.format.KML,
            ol.format.OSMXML
        ]
    });
    dragAndDrop.on('addfeatures', function (evt) {
        var source = new ol.source.Vector()
        var layer = new ol.layer.Vector({
            source: source,
            name: 'Drag&Drop Layer'
        });
        tree.addBufferIcon(layer);
        map.addLayer(layer);
        source.addFeatures(evt.features);
    });
    
    
    var mapserverURL = 'http://47.93.17.65';

    function getGTTileSource(layername) {
        var mapURL = mapserverURL + '/maps/gts/tms/v1/{mapName}/tile/{x}/{y}/{z}';
        mapURL = mapURL.replace('{mapName}', encodeURIComponent(layername));

        function checkTileXY_(ncol, nrow) {
            var x = (ncol & 0xffffff80) | (nrow & 0x0000007f);
            var y = (nrow & 0xffffff80) | (ncol & 0x0000007f);
            return [x, y];
        };

        /**计算函数 */
        var f = function (tileCoord, pixelRatio, projection) {
            if (!tileCoord) {
                return "";
            }
            var z = tileCoord[0] - 0;
            var ncol = tileCoord[1] - 0;
            var nrow = 0 - tileCoord[2] - 1;
            //var nrow = 0 - tileCoord[2];
            //console.log('ncol=%d, nrow=%d, z=%d', ncol, nrow, z);
            var xy = checkTileXY_(ncol, nrow);

            var s = mapURL.replace('{z}', z + '');
            s = s.replace('{x}', xy[0] + '');
            s = s.replace('{y}', xy[1] + '');
            return s;
        };


        var xyzSource = new ol.source.XYZ({
            /*projection: 'EPGS:3857',*/
            minZoom: 0,
            maxZoom: 14,
            tileUrlFunction: f
        });
        return xyzSource;

    }

    

    var maskVectorLayer = new ol.layer.Vector({
        source: new ol.source.Vector({
            format: new ol.format.GeoJSON({
                defaultDataProjection: 'EPSG:4326'
            }),
            url: '../res/world_capitals.geojson',
            attributions: [
                new ol.Attribution({
                    html: '庚图科技'
                })
            ],									
        }),
        name:'各国首都',
        style: new ol.style.Style({
                    image: new ol.style.RegularShape({
                        stroke: new ol.style.Stroke({
                            width: 2,
                            color: [6, 125, 34, 1]
                        }),
                        fill: new ol.style.Fill({
                            color: [255, 0, 0, 0.3]
                        }),
                        points: 5,
                        radius1: 5,
                        radius2: 8,
                        rotation: Math.PI
                    })
            })
    });


	
    var map = new ol.Map({
        target: 'map',
        layers: [
		/*
            new ol.layer.Tile({
                source: new ol.source.OSM(),
				name:'OpenStreetMap'
            }),
		
			   	
			new ol.layer.Tile({
				preload: Infinity,
				source: new ol.source.BingMaps({
						key: 'As1HiMj1PvLPlqc_gtM7AqZfBL8ZL3VrjaS3zIb22Uvb9WKhuJObROC-qUpa81U5',
						imagerySet: 'Aerial'
						// use maxZoom 19 to see stretched tiles instead of the BingMaps
						// "no photos at this zoom level" tiles
						// maxZoom: 19
					}),
				name: 'Bing'
			}),*/
     
            new ol.layer.Tile({source:getGTTileSource('全球影像'),name:'影像'}),
            new ol.layer.Tile({source:getGTTileSource('影像矢量'),name:'影像叠加'}),

            //new ol.layer.Tile({source:getGTTileSource('矢量瓦片'),name:'矢量'}),
			
            maskVectorLayer,
        ],
        controls: [
            //Define some new controls
            new ol.control.MousePosition({
                coordinateFormat: function (coordinates) {
                    var coord_x = coordinates[0].toFixed(3);
                    var coord_y = coordinates[1].toFixed(3);
					
					var hdms = ol.coordinate.toStringHDMS(ol.proj.transform(
            coordinates, 'EPSG:3857', 'EPSG:4326'));
					return hdms;            
                },
                target: 'coordinates'
            })
            
        ],
        view: new ol.View({
            center: [0, 0],
            zoom: 2
        }),
		interactions: ol.interaction.defaults().extend([
			dragAndDrop
		])
		
    });

    
	var tree = new layerTree({map: map, target: 'layertree', messages:
            'messageBar'});

            console.log('layers '+ map.getLayers().getLength());
    for(var idx = 0 ;idx<map.getLayers().getLength();++idx)
        tree.createRegistry(map.getLayers().item(idx));
			
			
	//增加本地图层关联
    document.getElementById('addvector_form').addEventListener('submit',
        function (evt) {
            evt.preventDefault();
            tree.addVectorLayer(this);
            this.parentNode.style.display = 'none';
        });

    document.getElementById('newvector_form').addEventListener('submit', function (evt) {
        evt.preventDefault();
        tree.newVectorLayer(this);
        this.parentNode.style.display = 'none';
    });
    
                    

    var tools = new toolBar({
        map: map,
        target: 'toolbar',
        layertree: tree,
    }).addControl(new ol.control.Zoom());

    /*
    var selected_feature = null;
    var selectAction = new ol.interaction.Select();
    selectAction.on('select',function(e){
            if(e.selected.length>0)
            {
                selected_feature = e.selected[0];
                var itempropsDiv = document.getElementById('itemprops');
                itempropsDiv.style.display = 'block';
            }else
            {
                alert('no item selected!');
                selected_feature = null;
            }
            
    });
    //矢量目标选中按钮
    tools.addControl(new ol.control.Interaction({
        interaction: selectAction
    }));


    //取消选中按钮
    var controlDiv = document.createElement('div');
    controlDiv.className = 'ol-deselect ol-unselectable ol-control';
    var controlButton = document.createElement('button');
    controlButton.title = '取消选取';
    controlDiv.appendChild(controlButton);
    controlButton.addEventListener('click', function () {
        selectAction.getFeatures().clear();
    });
    var deselectControl = new ol.control.Control({
        element: controlDiv
    });

    tools.addControl(deselectControl);
    */

    tools.addSelectControls();

    tools.addEditingToolBar();

    //二、三维视图切换按钮
    tools.addControl(new ol.control.Cesium({
        target: 'toolbar'
    }));


}
document.addEventListener('DOMContentLoaded', init);