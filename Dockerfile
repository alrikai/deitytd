FROM tiangolo/uvicorn-gunicorn-fastapi:python3.8-alpine3.10

RUN apk update && apk --no-cache --update add \ 
    curl \
    build-base \ 
    gcc \
    g++ \
    bash

# Install Poetry
RUN curl -sSL https://raw.githubusercontent.com/python-poetry/poetry/master/get-poetry.py | POETRY_HOME=/opt/poetry python && \
    cd /usr/local/bin && \
    ln -s /opt/poetry/bin/poetry && \
    poetry config virtualenvs.create false

COPY --from=deity-image:latest /deitytd/build/lib /libdtd

COPY ./data /data
COPY ./resources /resources
COPY ./script/docker-entrypoint.sh /docker-entrypoint.sh

# Copy using poetry.lock* in case it doesn't exist yet
COPY ./pyproject.toml ./poetry.lock* /
RUN poetry install --no-root --no-dev

COPY ./serve/app /app
ENTRYPOINT ["/docker-entrypoint.sh"]
