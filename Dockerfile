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

COPY ./src/token.txt /app/src/token.txt
CMD cat /app/src/token.txt

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


RUN groupadd -r sample && useradd -r -g sample sample
USER sample

WORKDIR /app

COPY --from=build /app/build/database .

ENTRYPOINT ["./database"]
