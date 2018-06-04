# Project information
AC_PREREQ([2.61])
AC_INIT([libguac-client-video], [0.1.0])
#AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_INIT_AUTOMAKE([-Wall  foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_SUBST(CPPFLAGS, "$CPPFLAGS -I/usr/include/cairo -I/home/liunan/ffmpeg_build/include")



AC_CONFIG_MACRO_DIRS([m4])

# Check for required build tools
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LIBTOOL



#PKG_CHECK_MODULES(AVCODEC, ,, AC_MSG_ERROR("libavcodec is required for communication via "
#                   "the x264 remote protocol"))

# Check for libguac
AC_CHECK_LIB([guac], [guac_client_stream_png],,
      AC_MSG_ERROR("libguac is required for communication via "
                   "the Guacamole protocol"))

AC_CHECK_LIB([cairo], [cairo_image_surface_create],,
      AC_MSG_ERROR("cairo_image_surface_create is required for rendering"))                   


#AC_CHECK_LIB([avcodec], [av_opt_set],,
#      AC_MSG_ERROR("avcodec for video stream"))      

AC_CONFIG_FILES([Makefile])
AC_OUTPUT