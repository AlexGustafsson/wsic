# Builder step
FROM ubuntu:18.04 as builder

# Install and configure dependencies
RUN apt update && \
  apt install -y software-properties-common && \
  add-apt-repository ppa:ubuntu-toolchain-r/test && \
  apt update && \
  apt install -y gcc-9 xxd make libssl-dev && \
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 1000 && \
  rm -rf /var/lib/apt/lists/*

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
ENV CC gcc

WORKDIR /wsic
COPY . /wsic
RUN make build

# Application step
FROM ubuntu:18.04

# Install and configure dependencies
RUN apt update && \
  apt install -y libssl1.1 && \
  rm -rf /var/lib/apt/lists/*

RUN useradd wsic

WORKDIR /wsic
COPY server.cert .
COPY server.key .
COPY --from=builder /wsic/build/wsic .
USER wsic
CMD ["./wsic"]
