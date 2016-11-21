#
# simple http responder based on node.js
#
# $Id: Dockerfile,v 1.1 2016/11/21 13:01:07 root Exp root $
#
FROM gliderlabs/alpine:3.4
WORKDIR /root
RUN apk add --no-cache curl nodejs \
        && npm install --save util dateformat express
COPY index.js /root/index.js
EXPOSE 80
CMD ["node", "/root/index.js"]
