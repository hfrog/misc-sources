
IMG=hfrog/eater
TAG=latest

.PHONY: all build push
all: build push

build: eater
	docker build -t $(IMG):$(TAG) .

push:
	docker push $(IMG):$(TAG)

eater: eater.c
	cc --static -std=c99 -o $@ $<

