# docker build -t hczhu.dev -f docker-images/dev.Dockerfile .
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
     || true
RUN apt clean && rm -rf /var/lib/apt/lists/*

RUN useradd -m ubuntu
RUN adduser ubuntu sudo
USER ubuntu


