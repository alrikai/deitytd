#!/bin/bash

docker build -t deity-image .

dtd_srcdir=$(pwd)
docker run -ti --rm -v "$dtd_srcdir/":/deitytd/ -e DISPLAY=$DISPLAY --device=/dev/dri:/dev/dri deity-image bash 
