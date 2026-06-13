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
        gnutls-dev \
        build-essential \
        cmake \
        make \
        bash \
        vim \
        sudo \
        git \
        clang \
        clang-format \
        ninja-build \
        pkg-config \
        libpq-dev \
        libyaml-cpp-dev \
        lld \
        python3-fire \
        lsb-release \
        software-properties-common
    
RUN wget https://apt.llvm.org/llvm.sh && \
        chmod +x llvm.sh && \
        sudo ./llvm.sh 21
# --- SQLITE EXTRAS ---
# RUN apt-get -o Acquire::Retries=5 -y install \
#         tcl8.6 tcl8.6-tdbc-sqlite3
    

# --- MySQL (and mariadb) build dependencies---

# RUN apt-get -o Acquire::Retries=5 install -y --no-install-recommends \
#         libmysqlclient-dev \
#         apt-transport-https \
#         software-properties-common
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