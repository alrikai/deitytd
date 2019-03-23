FROM ubuntu:latest

LABEL maintainer="Alrik Firl afirlortwo@gmail.com" \
      version="0.1" \
      description="DeityTD Dockerfile"

# Update packages
RUN apt-get update --fix-missing

# Install system tools / libraries
RUN apt-get -y install build-essential \
    ssh \
    sudo \
    vim \
    git \
    wget \
    make \
    cmake \
    virt-viewer \
    ffmpeg \
    libboost-all-dev \
    libopencv-dev \ 
    libogre-1.9-dev \
    libois-dev \ 
    libcegui-mk2-dev \ 
    ocl-icd-opencl-dev \ 
    libyaml-cpp-dev

#(userid): id -u alrik --> 1000, (groupid): id -g  alrik--> 1000 (this presumably has to be changed if not the 1st user on the system?)
RUN export uid=1000 gid=1000 devname=DTD && \
    mkdir -p /home/${devname} && \
    echo "${devname}:x:${uid}:${gid}:Developer,,,:/home/${devname}:/bin/bash" >> /etc/passwd && \
    echo "${devname}:x:${uid}:" >> /etc/group && \
    touch /etc/sudoers.d/${devname} && \
    echo "${devname} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${devname} && \
    chmod 0440 /etc/sudoers.d/${devname} && \
    chown ${uid}:${gid} -R /home/${devname} && \
    mkdir -p /home/DTD/deitytd/build && \
    chmod 7777 -R /home/DTD/deitytd

#RUN useradd -s /bin/bash DTD
USER DTD 
ENV HOME /home/DTD
WORKDIR /home/DTD

RUN wget https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp
RUN sudo cp cl.hpp /usr/include/CL/cl.hpp
RUN sudo ln -s /usr/include/cegui-0.8.4/CEGUI /usr/include/CEGUI

#COPY *.txt deitytd/
#COPY Controller deitytd/Controller
#COPY Testing deitytd/Testing
#COPY Common deitytd/Common
#COPY cmake deitytd/cmake
#COPY Model deitytd/Model

# Enable additional output from Launcher
#ENV QT_VERBOSE true
#ENV QT_TESTING true
# Xvfb
ENV DISPLAY :99

ENTRYPOINT cd deitytd/build; /bin/bash
