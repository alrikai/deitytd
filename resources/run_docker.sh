#!/bin/bash

docker build -t deity-image .

dtd_srcdir=$(pwd)
docker run -ti --rm --cap-add=SYS_PTRACE -u DTD -v "$dtd_srcdir/":/deitytd/ -e DISPLAY=$DISPLAY --device=/dev/dri:/dev/dri -v /tmp/.X11-unix:/tmp/.X11-unix deity-image bash 
