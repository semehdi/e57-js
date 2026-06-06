FROM gcc

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

WORKDIR /deps

RUN apt update
RUN apt install git python3 python3-pip wget cmake -y
RUN git clone https://github.com/emscripten-core/emsdk.git \
    && cd emsdk \
    && ./emsdk install 5.0.7 \
    && ./emsdk activate 5.0.7

ENV PATH="/deps/emsdk:${PATH}"
ENV PATH="/deps/emsdk/upstream/emscripten:${PATH}"
ENV PATH="/deps/emsdk/node/22.16.0_64bit/bin:${PATH}"

WORKDIR /app

CMD ["bash"]
