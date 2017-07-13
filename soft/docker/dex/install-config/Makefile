
IMG=hfrog/dex-install-config
TAG=v1.0

.PHONY: all build push
all: build push

build:
	docker build -t $(IMG):$(TAG) .

push:
	docker push $(IMG):$(TAG)

