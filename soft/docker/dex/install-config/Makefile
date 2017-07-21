
IMG=hfrog/dex-install-config
TAG=v1.1

.PHONY: all build push
all: build push

build:
	docker build -t $(IMG):$(TAG) .

push:
	docker push $(IMG):$(TAG)

