FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# System packages and C++ libraries
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    # C++ third-party libs (used by cpp/4fun/, ai-ml/mlight/, IBM-Ponder-This-solvers/)
    libgtest-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libfolly-dev \
    libarmadillo-dev \
    libopenblas-dev \
    liblapack-dev \
    libfmt-dev \
    libevent-dev \
    libbenchmark-dev \
    # Python
    python3 \
    python3-pip \
    python3-dev \
    python3-tk \
    && rm -rf /var/lib/apt/lists/*

# libgtest-dev ships sources only; build and install the static libraries
RUN cd /usr/src/googletest \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel "$(nproc)" \
    && cmake --install build

# numpy first so the torch layer can reuse it from cache
RUN pip3 install --no-cache-dir numpy

# PyTorch CPU-only (GPU passthrough requires nvidia-docker); kept in its own
# layer so rebuilds from later pip changes don't re-download ~800 MB
RUN pip3 install --no-cache-dir torch \
    --index-url https://download.pytorch.org/whl/cpu

# Remaining Python packages
RUN pip3 install --no-cache-dir \
    httpx \
    fire \
    sympy \
    tldextract \
    matplotlib \
    pytest \
    pytest-asyncio \
    pytest-xdist

# Go
ARG GO_VERSION=1.21.12
RUN wget -q "https://dl.google.com/go/go${GO_VERSION}.linux-amd64.tar.gz" -O /tmp/go.tar.gz \
    && tar -xzf /tmp/go.tar.gz -C /usr/local \
    && rm /tmp/go.tar.gz

ENV PATH="/usr/local/go/bin:/root/go/bin:${PATH}"
ENV GOPATH=/root/go

# Pre-download Go module dependencies so they are baked into the image
COPY golang/go.mod golang/go.sum /tmp/gomod/
RUN cd /tmp/gomod && go mod download

WORKDIR /workspace
