# DOCKER_BUILDKIT=1 docker build -t hczhu.dev -f docker-images/dev.Dockerfile .
FROM ubuntu

RUN (apt update || true ) \
  && (apt upgrade || true) \
  && (apt install update-manager-core || true)
RUN rm -rf /var/lib/apt/lists/*
RUN apt update || true 
RUN apt install -y \
    git \
    vim \
    iputils-ping \
    curl \
    sudo \
    tmux \
     || true
RUN apt clean && rm -rf /var/lib/apt/lists/*

RUN useradd -m ubuntu
RUN echo 'ubuntu:password' | chpasswd
RUN adduser ubuntu sudo
USER ubuntu

WORKDIR /home/ubuntu
RUN cd /home/ubuntu
RUN git clone https://github.com/hczhu/code_recipes.git
RUN cd code_recipes/dotfiles && (bash setup.sh || true)

SHELL ["/bin/bash", "-c"]
