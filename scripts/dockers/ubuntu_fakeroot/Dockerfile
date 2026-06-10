# -------------------------------------------------
# Dockerfile – Ubuntu 22.04 with fakeroot
# -------------------------------------------------
FROM ubuntu:22.04

# Set non‑interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install fakeroot and a few helpful tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        fakeroot \
        ca-certificates \
        curl \
        wget \
        gnupg \
        build-essential \
        cmake \
        git && \
    rm -rf /var/lib/apt/lists/*

# Create a regular user (optional but recommended)
ARG USERNAME=builder
ARG UID=1000
ARG GID=1000
RUN groupadd -g ${GID} ${USERNAME} && \
    useradd -m -u ${UID} -g ${GID} -s /bin/bash ${USERNAME}

# Switch to the non‑root user
USER ${USERNAME}
WORKDIR /home/${USERNAME}

# -----------------------------------------------------------------
# Example usage (run from host):
#   docker build -t ubuntu-fakeroot .
#   docker run --rm -it ubuntu-fakeroot bash
# Inside the container you can now do:
#   fakeroot <command>
# -----------------------------------------------------------------