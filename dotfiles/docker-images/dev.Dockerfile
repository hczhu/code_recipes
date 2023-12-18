# DOCKER_BUILDKIT=1 docker build -t hczhu.dev -f docker-images/dev.Dockerfile .
FROM ubuntu:latest

RUN (apt update || true ) \
  && (apt upgrade || true)
# RUN rm -rf /var/lib/apt/lists/*
RUN apt install -y \
    git \
    vim \
    iputils-ping \
    curl \
    sudo \
    tmux \
    wget \
    cron \
    python3-pip \
    systemd

RUN apt clean && rm -rf /var/lib/apt/lists/*

# Doesn't work with an error, "hostname: you must be root to change the host name"
# RUN sudo hostname hczhu.dev.container

# Golang
ARG GO_VERSION=1.21.5
RUN wget https://dl.google.com/go/go${GO_VERSION}.linux-amd64.tar.gz -O go.tar.gz \
    && tar -xzf go.tar.gz -C /usr/local \
        && rm go.tar.gz
ENV GOROOT=/usr/local/go
# ENV GOPATH=/home/ubuntu/
ENV GODEBUG madvdontneed=1
RUN /usr/local/go/bin/go install -v golang.org/x/tools/gopls@v0.14.2

RUN useradd -m ubuntu
RUN echo 'ubuntu:password' | chpasswd
RUN adduser ubuntu sudo
USER ubuntu

WORKDIR /home/ubuntu
RUN cd /home/ubuntu
RUN git clone https://github.com/hczhu/code_recipes.git
# Too much side effect
# RUN cd code_recipes/dotfiles && (bash setup.sh || true)

SHELL ["/bin/bash", "-c"]
