# docker build -t hczhu.dev -f docker-images/dev.Dockerfile docker-images/
FROM ubuntu:21.04

RUN apt update && \
  apt install \
    git-core
RUN git clone https://github.com/hczhu/dotfiles.git
