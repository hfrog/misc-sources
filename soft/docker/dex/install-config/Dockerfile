FROM gliderlabs/alpine:3.4

MAINTAINER Andrew Ivanov <and@hfrog.ru>

WORKDIR /root

RUN apk add --no-cache \
    bash \
    less

ADD Dockerfile /
ADD install-config.sh /

CMD ["/install-config.sh"]

