FROM gcc:latest as build

RUN apt-get update && \
    apt-get install -y \
    libboost-dev libboost-system-dev \
    cmake 

RUN apt-get install g++ make binutils libboost-system-dev libssl-dev zlib1g-dev libcurl4-openssl-dev

RUN git clone https://github.com/reo7sp/tgbot-cpp
RUN cmake ./tgbot-cpp
RUN cd tgbot-cpp
RUN make -j4
RUN make install
RUN cd ..

RUN apt-get install -y libpq-dev libpqxx-dev

# RUN  git clone https://github.com/jtv/libpqxx.git
# RUN cd libpqxx && \
#     ./configure --disable-shared && \
#     make && \
#     make install && \
#     cd ..

RUN git clone https://github.com/jtv/libpqxx.git \
    --branch 6.4 --depth 1 \
    && cd libpqxx/ && mkdir build && cd build/ && cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DPQXX_DIR=/usr/local/lib \ 
    -DPostgreSQL_DIR=/usr/lib/x86_64-linux-gnu \
    -DPQXX_TYPE_INCLUDE_DIR=/usr/local/include/pqxx \
    -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql \
    -DCMAKE_MODULE_PATH=/usr/src/libpqxx-r6.4/cmake .. \
    && make && make install && ldconfig /usr/local/lib 


RUN ls /usr/local/lib/

ADD ./src /app/src
WORKDIR /app/build

RUN cmake ../src && \
    cmake --build . 


FROM ubuntu:latest


RUN apt-get -y update && \
    apt-get install -y software-properties-common && \
    apt-get -y update 

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get -y update && \
    apt-get install -y gcc-4.9 && \
    apt-get upgrade -y libstdc++6 


RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --yes \
    libcurl4-openssl-dev libboost-system-dev\
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git make cmake g++ 

RUN apt-get install -y libpq-dev libpqxx-dev


RUN git clone https://github.com/jtv/libpqxx.git \
    --branch 6.4 --depth 1 \
    && cd libpqxx/ && mkdir build && cd build/ && cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DPQXX_DIR=/usr/local/lib \ 
    -DPostgreSQL_DIR=/usr/lib/x86_64-linux-gnu \
    -DPQXX_TYPE_INCLUDE_DIR=/usr/local/include/pqxx \
    -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql \
    -DCMAKE_MODULE_PATH=/usr/src/libpqxx-r6.4/cmake .. \
    && make && make install && ldconfig /usr/local/lib 

RUN find / -name libpq.so.5

RUN groupadd -r sample && useradd -r -g sample sample
USER sample

WORKDIR /app

COPY --from=build /app/build/database .

ENTRYPOINT ["./database"]
