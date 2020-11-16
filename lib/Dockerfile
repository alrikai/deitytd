FROM ubuntu:20.04

LABEL maintainer="Alrik Firl afirlortwo@gmail.com" \
      version="0.1" \
      description="DeityTD Builder Dockerfile"

ENV DEBIAN_FRONTEND=noninteractive
# Update packages
RUN apt-get update --fix-missing

# Install system tools / libraries
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    gcc \ 
	g++ \ 
    build-essential \
    git \
    make \
    cmake \
    ffmpeg \
    libboost-all-dev \
    cmake-data \
	scons \
	ccache 

#stuff for development & debugging
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    vim \
    sudo \
    gdb

#python things
RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
  python3-pip \
  python3.8 \
  python3.8-dev \
  python3.8-distutils \
  python3.8-venv
  
WORKDIR /

#RUN curl -sSL https://raw.githubusercontent.com/sdispater/poetry/master/get-poetry.py | python
RUN pip3 install --upgrade pip
RUN pip --no-cache-dir install poetry
COPY poetry.lock pyproject.toml ./
RUN poetry config virtualenvs.create false
#TODO: if we don't want to be able to run tests, --no-dev 
RUN poetry install --no-interaction --no-ansi 

# build a static lib of yaml-cpp
RUN git clone https://github.com/jbeder/yaml-cpp.git
RUN mkdir -p /yaml-cpp/build
WORKDIR /yaml-cpp/build
RUN CFLAGS="-fPIC" CXXFLAGS="-fPIC" cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4 && make install

WORKDIR /

RUN mkdir -p /dtddeps
RUN cp /yaml-cpp/build/libyaml-cpp.a /dtddeps/libyaml-cpp.a

RUN git clone https://github.com/alrikai/fflames.git
ENV PATH="/fflames:${PATH}"

COPY ./lib /deitytd
COPY ./data /deitytd/data
COPY ./resources /resources
COPY ./script/libdocker-entrypoint.sh /docker-entrypoint.sh
COPY ./script/test_lib.sh /test.sh

RUN mkdir -p /deitytdbuild
WORKDIR /deitytd/build
#TODO: make debug/release more easily configurable 
RUN cmake .. -DCMAKE_BUILD_TYPE=DEBUG && make -j4

WORKDIR /

#poetry build
#cd "$DTDDIR/Bindings"
#python3.8 setup.py develop
#cd "$DTDDIR"

#export PYTHONPATH=$PYTHONPATH:"$DTDDIR/build/lib"
ENTRYPOINT ["/docker-entrypoint.sh"] 
CMD ["bash", "./test.sh"]
