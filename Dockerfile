FROM ubuntu:20.04

LABEL maintainer="Alrik Firl afirlortwo@gmail.com" \
      version="0.1" \
      description="DeityTD Dockerfile"

RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
    curl \
    gcc \ 
	g++ \ 
    build-essential

RUN apt-get update --fix-missing && apt-get --fix-missing -y install \
  python3-pip \
  python3.8 \
  python3.8-dev \
  python3.8-distutils \
  python3.8-venv
  
# Install Poetry
RUN curl -sSL https://raw.githubusercontent.com/python-poetry/poetry/master/get-poetry.py | POETRY_HOME=/opt/poetry python3.8 && \
    cd /usr/local/bin && \
    ln -s /opt/poetry/bin/poetry && \
    poetry config virtualenvs.create false

COPY --from=deity-image:latest /deitytd/build/lib /libdtd

COPY ./data /data
COPY ./resources /resources
COPY ./script/docker-entrypoint.sh /docker-entrypoint.sh
COPY ./script/serve.sh /serve.sh

# Copy using poetry.lock* in case it doesn't exist yet
COPY ./pyproject.toml ./poetry.lock* /
RUN poetry install --no-root --no-dev

COPY ./serve/app /app
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["bash", "./serve.sh"]
