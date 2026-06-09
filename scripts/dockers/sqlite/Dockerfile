FROM ubuntu:22.04
LABEL maintainer="Squirrel"

ENV USE_LLM=1
ENV DEBIAN_FRONTEND=noninteractive

# common config
RUN apt-get -o Acquire::Retries=5 update
RUN apt-get -o Acquire::Retries=5 -y install make cmake build-essential vim sudo git tcl8.6 tcl8.6-tdbc-sqlite3 \
    clang ninja-build pkg-config clang-format \
    libpq-dev libyaml-cpp-dev lld llvm python3-fire curl wget

RUN mkdir -p /home && \
    groupadd dobigthing && \
    useradd -l -K UMASK=0000 -d /home -g dobigthing dobigthing && \
    chown dobigthing:dobigthing /home

RUN	echo "dobigthing:dobigthing" | chpasswd && usermod -a -G sudo dobigthing
RUN chmod +w /etc/sudoers && \
    echo "%dobigthing   ALL=(ALL:ALL)NOPASSWD:ALL" >> /etc/sudoers && \
    chmod -w /etc/sudoers

USER dobigthing
WORKDIR /home
ARG USE_REMOTE=0
# Always copy the build context to a temp location so the Dockerfile can
# either use the local context or perform a remote git clone depending on
# the `USE_REMOTE` build-arg.
COPY . /tmp/squirrel_context
RUN if [ "$USE_REMOTE" = "1" ]; then \
            git config --global http.version HTTP/1.1 && \
            git config --global submodule.fetchJobs 1 && \
            git clone --depth=1 https://github.com/s3team/Squirrel.git /home/Squirrel && \
            cd /home/Squirrel && for attempt in 1 2 3; do \
                git submodule update --init --recursive --depth 1 --jobs 1 && break; \
            done; \
        else \
            mkdir -p /home/Squirrel && cp -a /tmp/squirrel_context/. /home/Squirrel/; \
            rm -rf /home/Squirrel/AFLplusplus && \
            cd /home/Squirrel && git submodule update --init --recursive --depth 1 --jobs 1; \
        fi && \
            cd /home/Squirrel && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSQLITE=ON && \
            cmake --build build -j && \
            cd AFLplusplus/ && LLVM_CONFIG=llvm-config-14 make -j20 && \
            cd ../llama.cpp/ && cmake -B build && cmake --build build --config Release

RUN cd /home/Squirrel/srcs/llm_fixer/models && wget -O /models/qwen2.5-coder-1.5b-instruct-q4_0.gguf \
    "https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B-Instruct-GGUF/resolve/main/qwen2.5-coder-1.5b-instruct-q4_0.gguf"

ARG CFLAGS="-DSQLITE_MAX_LENGTH=128000000 -DSQLITE_MAX_SQL_LENGTH=128000000 -DSQLITE_MAX_MEMORY=25000000 -DSQLITE_PRINTF_PRECISION_LIMIT=1048576 -DSQLITE_DEBUG=1 -DSQLITE_MAX_PAGE_COUNT=16384"

RUN git clone https://github.com/sqlite/sqlite sqlite3/ && cd sqlite3 && mkdir bld

WORKDIR /home/sqlite3/bld

ENV CC=/home/Squirrel/AFLplusplus/afl-cc
ENV CXX=/home/Squirrel/AFLplusplus/afl-c++
RUN ../configure && make -j && make sqlite3.c

RUN $CC $CFLAGS -I. -c \
    /home/sqlite3/test/ossfuzz.c -o /home/sqlite3/test/ossfuzz.o
RUN $CC $CFLAGS -I. -c \
    /home/sqlite3/test/ossshell.c -o /home/sqlite3/test/ossshell.o

RUN $CXX $CXXFLAGS \
    /home/sqlite3/test/ossfuzz.o /home/sqlite3/test/ossshell.o -o /home/ossfuzz \
   ./sqlite3.o -ldl -pthread

WORKDIR /home/Squirrel/scripts/utils
ENTRYPOINT python3 run.py sqlite ../../data/fuzz_root/input
