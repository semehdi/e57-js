FROM gcc

ENV PATH="/deps/emsdk:${PATH}"
ENV PATH="/deps/emsdk/upstream/emscripten:${PATH}"
ENV PATH="/deps/emsdk/node/22.16.0_64bit/bin:${PATH}"

WORKDIR /deps

RUN apt update
RUN apt install git python3 python3-pip wget cmake -y
RUN git clone https://github.com/emscripten-core/emsdk.git \
    && cd emsdk \
    && ./emsdk install latest \
    && ./emsdk activate latest

WORKDIR /app

ENTRYPOINT [ "/bin/bash" ]
