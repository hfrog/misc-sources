FROM ubuntu
ADD Dockerfile /
#COPY Dockerfile /
RUN apt-get -y update \
        && apt-get -y install \
                bind9-host \
                bridge-utils \
                curl \
                dnsutils \
                iputils-ping \
                less \
                net-tools \
                openssl \
                patch \
                telnet \
                vim \
                wget
CMD bash
