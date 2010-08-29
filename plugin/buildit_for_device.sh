#!/bin/bash

if [ ! "$PalmPDK" ];then
PalmPDK=/opt/PalmPDK/
fi

PATH=$PATH:${PalmPDK}/arm-gcc/bin

CC="arm-none-linux-gnueabi-g++"

SYSROOT="${PalmPDK}/arm-gcc/sysroot"

INCLUDEDIR="${PalmPDK}/include"
LIBDIR="${PalmPDK}/device/lib"

CPPFLAGS="-I${INCLUDEDIR} -I${INCLUDEDIR}/SDL --sysroot=$SYSROOT"
LDFLAGS="-L${LIBDIR} -Wl,-rpath-link,${LIBDIR},-noinhibit-exec"
LIBS="-lSDL -lGLESv2 -lpdl"

$CC $CPPFLAGS $LDFLAGS $LIBS -o socket_plugin simple.cpp

