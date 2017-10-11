FROM gliderlabs/alpine:3.6

LABEL maintainer="Andrew Ivanov <and@hfrog.ru>"

WORKDIR /root

RUN apk add --no-cache \
    bash \
    curl \
    less \
    openssl

ADD Dockerfile /

RUN ["/bin/bash", "-c", "curl -LO https://storage.googleapis.com/kubernetes-release/release/$(curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt)/bin/linux/amd64/kubectl; chmod +x kubectl"]

ADD bootstrap.sh /
RUN ["/bin/bash", "-c", "chmod +x /bootstrap.sh"]
CMD ["/bootstrap.sh"]

