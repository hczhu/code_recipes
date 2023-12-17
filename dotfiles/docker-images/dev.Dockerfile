# docker build -t hczhu.dev -f docker-images/dev.Dockerfile docker-images/
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
     || true

RUN apt clean && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/hczhu/dotfiles.git




