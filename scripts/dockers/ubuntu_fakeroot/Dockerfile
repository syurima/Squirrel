# -------------------------------------------------
# Dockerfile – Ubuntu 22.04 with fakeroot and common development tools for building Squirrel in WCSS
# -------------------------------------------------
FROM ubuntu:22.04

# Set non‑interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install fakeroot and the common development tools
RUN apt-get update && \
    apt-get install -y \
        fakeroot \
        ca-certificates \
        curl \
        wget \
        gnupg \
        gnutls-dev \
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
        python3-fire \
        software-properties-common
    

# # --- MySQL (and mariadb) build dependencies---

RUN apt-get -o Acquire::Retries=5 install -y \
        libmysqlclient-dev \
        libncurses5-dev \
        zlib1g-dev \
        libssl-dev \
        libreadline-dev \
        apt-transport-https
# RUN sed -i 's/^# deb-src /deb-src /' /etc/apt/sources.list && \
#     apt-get -o Acquire::Retries=5 update &&  \
#     apt-get -o Acquire::Retries=5 -y build-dep mariadb-server

RUN  rm -rf /var/lib/apt/lists*
# -----------------------------------------------------------------
# Example usage (run from host):
#   docker build -t syurima/ubuntu-fakeroot:latest .
#   docker run --rm -it syurima/ubuntu-fakeroot:latest bash
# To push to Docker Hub:
#   docker push syurima/ubuntu-fakeroot:latest
# Inside the container you can now do:
#   fakeroot <command>
# -----------------------------------------------------------------