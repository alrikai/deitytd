FROM ubuntu:18.04

LABEL maintainer="Alrik Firl afirlortwo@gmail.com" \
      version="0.1" \
      description="DeityTD Dockerfile"

ENV DEBIAN_FRONTEND=noninteractive
# Update packages
RUN apt-get update --fix-missing

# Install system tools / libraries
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    gcc \ 
	g++ \ 
    build-essential \
    sudo \
    vim \
    git \
    wget \
    make \
    cmake \
    virt-viewer 

RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    ffmpeg \
	mercurial \
    libboost-all-dev \
    libopencv-dev \ 
    libois-dev \ 
    ocl-icd-opencl-dev \ 
    cmake-data \
	scons \
	ccache

RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    libyaml-cpp-dev \ 
	libfreetype6-dev \
	libgles2-mesa-dev \
	libxt-dev \
	libxaw7-dev \ 
	libsdl2-dev \
	zlib1g-dev \ 
	libzzip-dev \ 
	libsdl2-dev \ 
	libfreeimage-dev \ 
	libxaw7-dev \ 
    libxt-dev \ 
	libglu1-mesa-dev

# Cegui deps
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    libfreetype6-dev \
	libsilly-dev \
	libxml2-dev \
	libexpat1-dev \
	libglfw3-dev \
	libglew-dev \
	libglm-dev \
	libgl1-mesa-glx \
	libgl1-mesa-dri

#stuff for development & debugging
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
  gdb


RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
  python3-pip
  
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

# download conda
RUN ["/bin/bash", "-c", "wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O $HOME/miniconda.sh"]
RUN chmod 0755 $HOME/miniconda.sh
RUN ["/bin/bash", "-c", "$HOME/miniconda.sh -b -p $HOME/conda"]
ENV PATH="$HOME/conda/bin:$PATH"
RUN rm $HOME/miniconda.sh
RUN conda update conda

RUN wget https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp
RUN sudo cp cl.hpp /usr/include/CL/cl.hpp
#RUN sudo ln -s /usr/include/cegui-0.8.4/CEGUI /usr/include/CEGUI

#RUN hg clone https://bitbucket.org/cegui/cegui -u v0-8
#RUN hg clone https://bitbucket.org/sinbad/ogre/ -u v1-9
#COPY resources/ogre-find-patch.diff cegui/.

#RUN mkdir ogre/build && cd ogre/build && cmake .. && make -j2 && sudo make install
#RUN cd cegui && hg import ogre-find-patch.diff --no-commit
#RUN mkdir cegui/build && cd cegui/build && cmake .. && make -j2 && sudo make install

RUN git clone https://github.com/alrikai/fflames.git
RUN sudo cp -r fflames/fflames /usr/local/include/FractalFlames

RUN conda create -n deitytd-env python=3.7 ipython 

# Enable additional output from Launcher
#ENV QT_VERBOSE true
#ENV QT_TESTING true
# Xvfb
ENV DISPLAY :99

COPY ./resources/docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"] 

CMD ["/bin/bash"]
