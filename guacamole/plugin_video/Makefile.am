AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall -pedantic

lib_LTLIBRARIES = libguac-client-video.la

# All source files of libguac-client-ball
libguac_client_video_la_SOURCES = src/videosender.c \
				src/bs_array.c \
				src/bs_vector.c \
				src/bs_pointmass.c \
				src/bs_joint.c \
    			src/bs_stick.c \
				src/bs_env.c \
				src/bs_blob.c \
				src/bs_blob_collective.c \
				src/bs_util.c \
				src/bs_list.c \
				src/bs_octree.c \
				src/bs_vector_util.c \
				src/bs_profiler.c \
				src/bs_rubberband.c \
				src/bs_ln_cairo.c



# libtool versioning information
libguac_client_video_la_LDFLAGS = -version-info 0:0:0
