FROM ubuntu:24.04
ARG TOOLS_DEB=machine-guest-tools_riscv64.deb
ADD ${TOOLS_DEB} /tmp/
RUN apt-get update && \
    apt-get upgrade -y --no-install-recommends && \
    useradd --create-home --user-group dapp && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        busybox-static \
        coreutils \
        bash \
        psmisc \
        bc \
        curl \
        device-tree-compiler \
        jq \
        lua5.4 \
        lua-socket \
        xxd \
        file \
        strace \
        stress-ng \
        libatomic1 \
        /tmp/machine-guest-tools_riscv64.deb \
    && \
    rm -rf /var/lib/apt/lists/* /var/cache/* /tmp/${TOOLS_DEB}
