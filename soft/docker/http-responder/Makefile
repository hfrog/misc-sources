
IMG=hfrog/http-responder
TAG=latest

.PHONY: all build push
all: build push

build:
	docker build -t $(IMG):$(TAG) .

push:
	docker push $(IMG):$(TAG)

