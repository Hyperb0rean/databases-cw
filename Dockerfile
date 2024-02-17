FROM gcc:latest as build

RUN apt-get update && \
    apt-get install -y \
    libboost-dev libboost-program-options-dev \
    cmake 


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

RUN groupadd -r sample && useradd -r -g sample sample
USER sample

WORKDIR /app

COPY --from=build /app/build/database .

ENTRYPOINT ["./database"]
