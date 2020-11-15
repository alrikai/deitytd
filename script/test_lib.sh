#!/bin/bash

pushd /deitytd/build 
make test
popd 

pushd /deitytd
pytest test/BindingsTest
popd
