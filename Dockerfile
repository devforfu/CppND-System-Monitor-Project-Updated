FROM docker-public.docker.devstack.vwgroup.com/ubuntu:20.04

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y \
    build-essential \
    python3 \
    python3-pip \
    git \
    gdb \
    bash \
    clang-tidy-10 \
    libncurses5-dev \
    libncursesw5-dev \
 && apt-get -y autoremove \
 && apt-get clean autoclean

RUN pip3 install --upgrade pip && \
    pip3 install \
    cmake==3.20.* \
    clang-format==10.0.*

RUN mkdir /repo
WORKDIR /repo

CMD ["/bin/bash"]

