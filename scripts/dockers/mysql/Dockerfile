FROM ubuntu:22.04
LABEL maintainer="Squirrel"

ENV USE_LLM=1
ENV DEBIAN_FRONTEND=noninteractive

# common config (done in batches to help with caching)
RUN apt-get -o Acquire::Retries=5 update
RUN apt-get -o Acquire::Retries=5 -y install make cmake build-essential vim sudo git
RUN apt-get -o Acquire::Retries=5 -y install clang libmysqlclient-dev ninja-build pkg-config clang-format
RUN apt-get -o Acquire::Retries=5 -y install libpq-dev libyaml-cpp-dev lld llvm bison python3-fire curl wget

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

ARG MYSQL_REF=mysql-8.4.9

# Install packages for MySQL build
RUN sudo apt-get -o Acquire::Retries=5 -y install apt-transport-https software-properties-common gnutls-dev
    
# Fetch and build boost 1.81 for MySQL (moved before MySQL source fetch to help with caching)
RUN set -eux; \
    for attempt in 1 2 3; do \
        git config --global http.version HTTP/1.1 && \
            git config --global submodule.fetchJobs 1 && \
            git clone --depth 1 --single-branch --branch "${MYSQL_REF}" \
            https://github.com/mysql/mysql-server.git /home/mysql && break; \
        rm -rf /home/mysql; \
        sleep $((attempt * 2)); \
    done;

ARG USE_REMOTE=0
ARG BUILD_JOBS=20
# Always copy the build context to a temp location so the Dockerfile can
# either use the local context or perform a remote git clone depending on
# the `USE_REMOTE` build-arg.
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
RUN cd /home/Squirrel && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMYSQL=ON && \
    cmake --build build --parallel "${BUILD_JOBS}" && \
    cd AFLplusplus/ && LLVM_CONFIG=llvm-config-14 make -j"${BUILD_JOBS}" && \
    cd ../llama.cpp/ && cmake -B build && cmake --build build --config Release

RUN cd /home/Squirrel/srcs/llm_fixer/models && wget -O /models/qwen2.5-coder-1.5b-instruct-q4_0.gguf \
    "https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B-Instruct-GGUF/resolve/main/qwen2.5-coder-1.5b-instruct-q4_0.gguf"

RUN CC=/home/Squirrel/AFLplusplus/afl-clang-fast CXX=/home/Squirrel/AFLplusplus/afl-clang-fast++ \
    cmake -S /home/mysql -B /home/bld -DDOWNLOAD_BOOST=1 -DWITH_BOOST=/home/boost \
    -DWITH_DEBUG=1 -DCPACK_MONOLITHIC_INSTALL=1 -DWITH_UNIT_TESTS=OFF
    
RUN cmake --build /home/bld --parallel "${BUILD_JOBS}" && sudo cmake --install /home/bld --prefix /usr/local/mysql/

RUN sudo chown dobigthing:dobigthing /usr/local/mysql/ -R && \
    cd /usr/local/mysql/ && mkdir mysql-files && chmod 750 mysql-files && \
    AFL_IGNORE_PROBLEMS=1 bin/mysqld --initialize-insecure --user=dobigthing && \
    if [ -x bin/mysql_ssl_rsa_setup ]; then bin/mysql_ssl_rsa_setup; fi

#ENTRYPOINT bash
#ENTRYPOINT /usr/local/mysql/bin/mysqld --basedir=/usr/local/mysql \
#            --datadir=/usr/local/mysql/data --log-error=err_log.err \
#            --pid-file=server_pid.pid --max_statement_time=1
RUN AFL_DEBUG=1 __AFL_SHM_ID=1234 /usr/local/mysql/bin/mysqld 2>&1 | grep "__afl_map_size" | tail -n 1 | cut -d"," -f8 | cut -d" " -f 3 > /tmp/mapsize
WORKDIR /home/Squirrel/scripts/utils
ENTRYPOINT AFL_IGNORE_PROBLEMS=1 AFL_MAP_SIZE=$(cat /tmp/mapsize) python3 run.py mysql ../../data/fuzz_root/mysql_input/
