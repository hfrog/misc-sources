FROM gliderlabs/alpine:3.4
WORKDIR /root
ADD Dockerfile /
#COPY Dockerfile /
RUN apk add --no-cache \
        bash \
        bind-tools \
        curl \
        heimdal-telnet \
        iputils \
        less \
        openssh-client \
        openssl \
        patch \
        tcpdump \
        vim \
        wget
RUN mv /usr/bin/telnet /usr/bin/telnet-stock
RUN ln -s /usr/bin/ktelnet /usr/bin/telnet

CMD bash
