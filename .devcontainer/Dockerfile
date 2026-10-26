FROM mcr.microsoft.com/devcontainers/cpp:2-debian13@sha256:8eaa3e4cf45fc14f2e1c6b5e668813168a42538716ce0b96e4752bf86fc1268f

# install all required dependencies.
RUN apt-get update && apt-get install -y python3 python3-pip unzip zstd curl && apt-get -y autoremove && apt-get -y autoclean


# Docker CLI only: the engine runs in the docker:dind-rootless service.
COPY --from=docker:29.6-cli@sha256:be132a9f282288de4afaf63379dff75711fda0147c6b72a9df44e51841402144 /usr/local/bin/docker /usr/local/bin/docker

ENV DOCKER_HOST=tcp://docker:2376
ENV DOCKER_TLS_VERIFY=1
ENV DOCKER_CERT_PATH=/certs/client
# VOLUME /certs/ca
VOLUME /certs/client
