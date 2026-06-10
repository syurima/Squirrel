# -------------------------------------------------
# Dockerfile – Ubuntu 22.04 with fakeroot and common development tools for building Squirrel in WCSS
# -------------------------------------------------
FROM ubuntu:22.04

# Set non‑interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install fakeroot and the common development tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        fakeroot \
        ca-certificates \
        curl \
        wget \
        gnupg \
        build-essential \
        cmake \
        make \
        vim \
        sudo \
        git \
        clang \
        clang-format \
        ninja-build \
        pkg-config \
        libpq-dev \
        libyaml-cpp-dev \
        llvm \
        lld \
        python3-fire && \
    rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------
# Example usage (run from host):
#   docker build -t ubuntu-fakeroot .
#   docker run --rm -it ubuntu-fakeroot bash
# Inside the container you can now do:
#   fakeroot <command>
# -----------------------------------------------------------------