#!/bin/bash

set -e

exec uvicorn --host 0.0.0.0 --port 8000 "app.main:app" 
