#building bot

FROM gcc:latest as build

RUN apt-get update && \
    apt-get install -y \
    libboost-dev libboost-system-dev \
    cmake 


RUN apt-get install g++ make binutils libboost-system-dev libssl-dev zlib1g-dev libcurl4-openssl-dev

#installing tgbot lib

RUN git clone https://github.com/reo7sp/tgbot-cpp
RUN cmake ./tgbot-cpp
RUN cd tgbot-cpp
RUN make -j4
RUN make install
RUN cd ..


#installing libpqxx

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

#building sources

ADD ./src /app/src
WORKDIR /app/build

RUN cmake ../src && \
    cmake --build . 

#run configuration

FROM ubuntu:latest

#installing base dependencies

RUN apt-get -y update && \
    apt-get install -y software-properties-common && \
    apt-get -y update 

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN   apt-get -y update && \
    apt-get install -y gcc-4.9 && \
    apt-get upgrade -y libstdc++6 


RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --yes \
    libcurl4-openssl-dev libboost-system-dev\
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git make cmake g++ 

#installing libpqxx

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

#installing postgresql

RUN apt-get install -y locales locales-all
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8
RUN localedef -i en_GB -c -f UTF-8 -A /usr/share/locale/locale.alias en_GB.UTF-8
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata
RUN apt-get -y install wget ca-certificates && \
    wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add - && \
    sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt/ `lsb_release -cs`-pgdg main" >> /etc/apt/sources.list.d/pgdg.list' &&\
    apt-get -y update
RUN apt-get -y install postgresql postgresql-contrib

RUN /etc/init.d/postgresql start
RUN echo "host all all 0.0.0.0/0 md5" >>/etc/postgresql/16/main/pg_hba.conf && \
    echo "listen_addresses='*'" >> /etc/postgresql/16/main/postgresql.conf 
RUN  /etc/init.d/postgresql restart

WORKDIR /app
COPY --from=build /app/build/database .
COPY --from=build /app/src .
RUN ls -la
RUN chmod +x ./run.sh

#changing rights

RUN groupadd -r sample && useradd -r -g sample sample
USER sample
USER postgres
RUN pg_lsclusters
ENV PGPASSWORD=postgres

# creating database

RUN  pg_ctlcluster 16 main start && \
    createdb mintdb
RUN pg_lsclusters

#running

EXPOSE 5432

ENTRYPOINT ["./run.sh"]
# RUN  pg_ctlcluster 16 main start
