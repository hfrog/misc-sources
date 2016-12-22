FROM gliderlabs/alpine:3.4
WORKDIR /root
ADD Dockerfile /
#COPY Dockerfile /
RUN apk add --no-cache \
        bash \
        bind-tools \
        curl \
        iputils \
        less \
        openssh-client \
        openssl \
        patch \
        tcpdump \
        vim \
        wget

CMD bash
