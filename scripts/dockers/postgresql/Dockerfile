FROM ubuntu:22.04
LABEL maintainer="Squirrel"

ENV DEBIAN_FRONTEND=noninteractive

# common config
RUN apt-get -o Acquire::Retries=5 update
RUN apt-get -o Acquire::Retries=5 -y install make cmake build-essential vim sudo git \
    clang ninja-build pkg-config clang-format libpq-dev llvm libyaml-cpp-dev zlib1g-dev \
    libreadline8 libreadline-dev bison flex python3-fire

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
ARG BUILD_JOBS=20
COPY . /tmp/squirrel_context
RUN if [ "$USE_REMOTE" = "1" ]; then \
            git config --global http.version HTTP/1.1 && \
            git config --global submodule.fetchJobs 1 && \
            git clone --depth=1 https://github.com/s3team/Squirrel.git /home/Squirrel; \
        else \
            mkdir -p /home/Squirrel && cp -a /tmp/squirrel_context/. /home/Squirrel/; \
            rm -rf /home/Squirrel/AFLplusplus; \
        fi 
RUN cd /home/Squirrel && for attempt in 1 2 3; do \
        git submodule update --init --depth 1 --jobs 1 && break; \
    done
RUN cd /home/Squirrel && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPOSTGRESQL=ON && \
    cmake --build build -j && \
    cd AFLplusplus/ && LLVM_CONFIG=llvm-config-14 make -j20

RUN cd /home/Squirrel && for attempt in 1 2 3; do \
        git config --global http.version HTTP/1.1 && \
        git config --global submodule.fetchJobs 1 && \
        git clone https://github.com/postgres/postgres.git postgres/ && break; \
    done; 
    
RUN mkdir bld/ && cd bld/ && \
    CC=/home/Squirrel/AFLplusplus/afl-cc CXX=/home/Squirrel/AFLplusplus/afl-c++ \
    /home/Squirrel/postgres/configure

RUN cd bld/ && make -j"${BUILD_JOBS}" && sudo make install && \
    sudo mkdir -p /usr/local/pgsql/data && sudo chown dobigthing /usr/local/pgsql/data &&  \
    AFL_IGNORE_PROBLEMS=1 /usr/local/pgsql/bin/initdb -D /usr/local/pgsql/data

RUN AFL_DEBUG=1 __AFL_SHM_ID=1234 /usr/local/pgsql/bin/postgres 2>&1 | grep "__afl_map_size" | tail -n 1 | cut -d"," -f8 | cut -d" " -f 3 > /tmp/mapsize
WORKDIR /home/Squirrel/scripts/utils
ENTRYPOINT AFL_MAP_SIZE=$(cat /tmp/mapsize) python3 run.py postgresql ../../data/fuzz_root/pqsql_input/
