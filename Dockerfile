# Copyright Cartesi and individual authors (see AUTHORS)
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

FROM ubuntu:24.04 AS tools-env

# Install dependencies
ARG LINUX_HEADERS_URLPATH=
ARG LINUX_HEADERS_SHA256=
ADD ${LINUX_HEADERS_URLPATH} /tmp/linux-libc-dev-riscv64-cross.deb
RUN <<EOF
set -e
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get upgrade -y
apt-get install -y --no-install-recommends \
        dpkg-dev \
        g++ \
        gcc \
        make \
        ca-certificates \
        git \
        libclang-dev \
        pkg-config \
        dpkg-cross \
        adduser \
        rustup \
        gcc-riscv64-linux-gnu \
        g++-riscv64-linux-gnu

echo "${LINUX_HEADERS_SHA256}  /tmp/linux-libc-dev-riscv64-cross.deb" | sha256sum --check
apt-get install -y --no-install-recommends --allow-downgrades /tmp/linux-libc-dev-riscv64-cross.deb
EOF

ENV TOOLCHAIN_PREFIX="riscv64-linux-gnu-"
ENV RUSTUP_HOME=/opt/rust
ENV PATH="/opt/rust/toolchains/1.77-x86_64-unknown-linux-gnu/bin:${PATH}"

# Install rust
RUN rustup default 1.77 && rustup target add riscv64gc-unknown-linux-gnu

# build
# ------------------------------------------------------------------------------
FROM tools-env AS builder

COPY . /work
WORKDIR /work

# Compile
RUN make -j$(nproc) libcmt
RUN make -j$(nproc) sys-utils
RUN make -j$(nproc) rollup-http

# Install locally
RUN make install DESTDIR=$(pwd)/_install PREFIX=/usr

# Strip
RUN <<EOF
set -e
riscv64-linux-gnu-strip _install/usr/bin/rollup-http-server
riscv64-linux-gnu-strip _install/usr/bin/echo-dapp
riscv64-linux-gnu-strip _install/usr/bin/rollup
riscv64-linux-gnu-strip _install/usr/bin/ioctl-echo-loop
riscv64-linux-gnu-strip _install/usr/bin/yield
riscv64-linux-gnu-strip _install/usr/bin/hex
riscv64-linux-gnu-strip _install/usr/sbin/xhalt
riscv64-linux-gnu-strip -S -x _install/usr/lib/*.so
riscv64-linux-gnu-strip -S _install/usr/lib/*.a
EOF

# Compile .tar.gz
ARG TOOLS_TARGZ=machine-guest-tools_riscv64.tar.gz
RUN cd _install && tar -czf /work/${TOOLS_TARGZ} *

# Compile .deb
ARG TOOLS_DEB=machine-guest-tools_riscv64.deb
RUN <<EOF
set -e
make control
mkdir _install/DEBIAN
cp control copyright postinst _install/DEBIAN/
dpkg-deb -Zxz --root-owner-group --build _install ${TOOLS_DEB}
EOF
