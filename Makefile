
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
		--security-opt seccomp=unconfined \
		-e DISPLAY=$(DISPLAY) \
		-v /dev:/dev \
		-v /tmp/.X11-unix:/tmp/.X11-unix \
		-v "$(PWD):/project" \
		$(IMAGE)
	xhost -local:docker

shell: run

clean:
	-docker rm -f $(CONTAINER)
	-docker image rm $(IMAGE)
