#!/bin/sh
echo 'Content-type: text/plain'
echo
#env
echo [$(date +'%b %d %T')] Request from ${REMOTE_ADDR}:${REMOTE_PORT} processed by host $(hostname) ${SERVER_ADDR}:${SERVER_PORT}
