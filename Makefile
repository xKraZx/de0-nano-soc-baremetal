
IMAGE := de0-nano-soc-baremetal
CONTAINER := de0-nano-soc-baremetal

.PHONY: all build run shell clean

all: build

build:
	docker buildx build \
		--load \
		-t $(IMAGE) \
        -f Dockerfile \
        .

run:
	xhost +local:docker
	docker run --rm -it \
		--privileged \
		--device=/dev/loop0 \
		--security-opt seccomp=unconfined \
		-e DISPLAY=$(DISPLAY) \
		-v /tmp/.X11-unix:/tmp/.X11-unix \
		-v "$(PWD):/project" \
		$(IMAGE)
	xhost -local:docker

shell: run

clean:
	-docker rm -f $(CONTAINER)
	-docker image rm $(IMAGE)
