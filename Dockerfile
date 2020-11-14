FROM ubuntu:20.04

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
    git \
    wget \
    make \
    cmake \
    ffmpeg \
    libboost-all-dev \
    libopencv-dev \ 
    ocl-icd-opencl-dev \ 
    cmake-data \
	scons \
	ccache \
    libyaml-cpp-dev \
    autoconf \
    libtool \
    pkg-config

#stuff for development & debugging
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    vim \
    gdb

#python things
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
  python3-pip \
  python3.8 \
  python3.8-dev \
  python3.8-distutils \
  python3.8-venv
  
#RUN curl -sSL https://raw.githubusercontent.com/sdispater/poetry/master/get-poetry.py | python
RUN pip3 install --upgrade pip
RUN pip --no-cache-dir install poetry
COPY poetry.lock pyproject.toml ./
RUN poetry config virtualenvs.create false
RUN poetry install --no-interaction --no-ansi 
#--no-dev 

#install opencl
RUN wget https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp
RUN sudo cp cl.hpp /usr/include/CL/cl.hpp

RUN git clone --recurse-submodules -b v1.31.0 https://github.com/grpc/grpc
RUN cd grpc && mkdir -p cmake/build && pushd cmake/build
RUN cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
RUN make -j && make install && popd

#(userid): id -u alrik --> 1000, (groupid): id -g  alrik--> 1000 (this presumably has to be changed if not the 1st user on the system?)

#RUN export uid=1000 gid=1000 devname=DTD && \
#    mkdir -p /home/${devname} && \
#    echo "${devname}:x:${uid}:${gid}:Developer,,,:/home/${devname}:/bin/bash" >> /etc/passwd && \
#    echo "${devname}:x:${uid}:" >> /etc/group && \
#    touch /etc/sudoers.d/${devname} && \
#    echo "${devname} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${devname} && \
#    chmod 0440 /etc/sudoers.d/${devname} && \
#    chown ${uid}:${gid} -R /home/${devname} && \
#    mkdir -p /home/DTD/deitytd/build && \
#    chmod 7777 -R /home/DTD/deitytd

#USER DTD 
#ENV HOME /home/DTD
#WORKDIR /home/DTD
WORKDIR /

RUN git clone https://github.com/alrikai/fflames.git
ENV PATH="/fflames:${PATH}"
#RUN sudo cp -r fflames/fflames /usr/local/include/FractalFlames

COPY ./script/docker-entrypoint.sh /.
ENTRYPOINT ["/docker-entrypoint.sh"] 

CMD ["/bin/bash"]
