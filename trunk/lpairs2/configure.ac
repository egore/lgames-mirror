#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(lpairs2, 2.2)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

AC_USE_SYSTEM_EXTENSIONS
AM_GNU_GETTEXT([external])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(m, main,, AC_MSG_ERROR(lib math is needed))
AC_CHECK_LIB(SDL2, main,, AC_MSG_ERROR(lib SDL2 is needed))
AC_CHECK_LIB(SDL2_ttf, main,, AC_MSG_ERROR(lib SDL2 ttf is needed))
AC_CHECK_LIB(SDL2_image, main,, AC_MSG_ERROR(lib SDL2 image is needed))
AC_CHECK_LIB(SDL2_mixer, main,, AC_MSG_ERROR(lib SDL2 mixer is needed))

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset strchr strdup strrchr])

# Flags
datdir=$datadir/$PACKAGE
configdir="~"
debugflags=""
AC_ARG_ENABLE( install,
[  --disable-install       No installation. Start from source directory.], 
datdir="." configdir=".")
AC_ARG_ENABLE( debug,
[  --enable-debug       Add debug symbols, no optimization.], 
debugflags="-O0 -g")
AC_SUBST(datdir)

SDL_CFLAGS=$(sdl2-config --cflags)
SDL_LIBS=$(sdl2-config --libs)

CXXFLAGS="$CXXFLAGS $SDL_CFLAGS -Wall $debugflags"
CXXFLAGS="$CXXFLAGS -std=c++11 -DLOCALEDIR=\\\"$localedir\\\" -DCONFIGDIR=\\\"$configdir\\\" -DDATADIR=\\\"$datdir\\\""
LIBS="$LIBS $SDL_LIBS"

# Makefiles
AC_OUTPUT(
Makefile 
po/Makefile.in
src/Makefile 
src/themes/Makefile 
src/themes/Standard/Makefile
src/themes/Standard/backgrounds/Makefile
src/themes/Standard/motifs/Makefile
)

