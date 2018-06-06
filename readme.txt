cd /usr/local/tomcat7

sudo ./bin/startup.sh
sudo ./bin/shutdown.sh















ffmpeg 编译

PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure \
  --prefix="$HOME/ffmpeg_build" \
  --pkg-config-flags="--static" \
  --disable-static \
  --enable-shared \
  --enable-pic \
  --extra-cflags="-I$HOME/ffmpeg_build/include" \
  --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
  --extra-libs="-lpthread -lm" \
  --bindir="$HOME/bin" \
  --enable-gpl \
  --enable-libx264 && \
PATH="$HOME/bin:$PATH" make && \
make install && \
hash -r

重新登录会使用　　
source ~/.profile　注册环境变量

---------------------------
ftyp  文件类型
moov  视频
mvhd　视频头部
trak　轨道数据
tkhd　轨道数据头部

mdia　媒体
mdhd　媒体头部
hdlr　处理引用
minf　Media Information Box
stbl　Sample Table Box
stsd　Sample Description Box
avc1　
mp4a　
esds　Elementary Stream Descriptor
avcC　AVC Configuration Box
btrt
stts
stsc	
stsz
stco
smhd
mdat


编码阶段
avcodec_send_frame
avcodec_receive_packet
av_packet_unref

//初始化阶段
AVCodec,AVCodecContext，AVFrame，AVPacket

avcodec_find_encoder_by_name
avcodec_alloc_context3
av_packet_alloc
av_opt_set



--------------------------------
These values and parameters, syntax elements, are converted into binary codes using vari-
able length coding and/or arithmetic coding. Each of these encoding methods produces an
efficient, compact binary representation of the information. The encoded bitstream can then
be stored and/or transmitted.

那些数据是参数，语法要素，使用变长或数学编码算法转换为二进制编码。


""

-r 30000/1001 -b:a 2M -bt 4M -vcodec libx264 -pass 1 -coder 0 -bf 0 -flags -loop -wpredp 0 -an recode.h264
sample useage
ffmpeg -y -i sourceFile -r 30000/1001 -b:a 2M -bt 4M -vcodec libx264 -pass 1 -coder 0 -bf 0 -flags -loop -wpredp 0 -an targetFile.mp4
-----------------------------------------

ffmpeg -i 3d.mp4 -ss 00:00:00 -t 00:00:20 -vcodec libx264 -profile:v baseline -pix_fmt yuv420p -s 800x450 gis.s.baseline.h264

将图片转换为视频
ffmpeg -f image2 -i 3d%03d.png -vcodec libx264 -profile:v baseline -pix_fmt yuv420p  gis.baseline.h264

分离出视频流
ffmpeg -i input_file -vcodec copy -an output_file_video　　//分离视频流


mvn -Drat.skip=true package
cp target/guacamole-example-0.9.14.war /usr/local/tomcat7/webapps/

ffmpeg_encode_video from rgb2yuv420 compile
gcc encode_video.c `pkg-config libavcodec libswscale libavutil --libs --cflags` -o encode_video


