OBJECTS = bs_main.o \
    bs_vector.o \
    bs_pointmass.o \
    bs_joint.o \
    bs_stick.o \
    bs_env.o \
    bs_array.o \
    bs_blob.o \
    bs_blob_collective.o \
    bs_cairo.o \
    bs_util.o \
    bs_list.o \
    bs_octree.o \
    bs_vector_util.o \
    bs_profiler.o \
    bs_rubberband.o

CC = gcc 

DISTDIR = ./bin

CFLAGS = `pkg-config --cflags cairo; sdl-config --cflags`
LIBS = `pkg-config --libs cairo; sdl-config --libs` -lm -lGL -lGLU

all: $(OBJECTS)
	$(CC) -g -o blobsallad $(OBJECTS) $(LIBS)


.c.o:
	$(CC) -g -Wall $(CFLAGS) $ -c $<

clean: 
	rm -f *.o; rm -f blobsallad; 

dist: 
	mkdir  $(DISTDIR); \
	cp *.c $(DISTDIR); \
	cp *.h $(DISTDIR); \
	cp Makefile $(DISTDIR); \
	cp -R maps $(DISTDIR); \
	tar -cvvf $(DISTDIR).tar $(DISTDIR); \
	gzip -c $(DISTDIR).tar > $(DISTDIR).tar.gz; \
	mv $(DISTDIR).tar.gz $(DISTDIR)-`date +"%F-%k-%M"`.tar.gz; \
	rm -rf $(DISTDIR); \
	rm -rf $(DISTDIR).tar;
