#!/bin/bash

docker run -ti --rm --cap-add=SYS_PTRACE -u DTD -v /home/alrik/Projects/deitytd/:/deitytd/ -e DISPLAY=$DISPLAY --device=/dev/dri:/dev/dri -v /tmp/.X11-unix:/tmp/.X11-unix deity-image bash
