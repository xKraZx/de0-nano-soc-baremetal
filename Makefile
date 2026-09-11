
IMAGE := de0-nano-soc-baremetal
CONTAINER := de0-nano-soc-baremetal

.PHONY: all build run shell sdimage clean

all: build

build:
	docker buildx build \
		--platform=linux/amd64 \
		--load \
		-t $(IMAGE) \
        -f Dockerfile \
        .

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    DISPLAY_ARGS := -e DISPLAY=host.docker.internal:0
    X11_ARGS :=
    XHOST_CMD := xhost +localhost
    XHOST_RESET_CMD := xhost -localhost
    JAVA_ARGS := -e _JAVA_OPTIONS="-Dsun.java2d.xrender=false"
else
    DISPLAY_ARGS := -e DISPLAY=$(DISPLAY)
    X11_ARGS := -v /dev:/dev \
                 -v /tmp/.X11-unix:/tmp/.X11-unix
    XHOST_CMD := xhost +local:docker
    XHOST_RESET_CMD := xhost -local:docker
    JAVA_ARGS :=
endif

run:
	$(XHOST_CMD)
	docker run --rm -it \
		--platform=linux/amd64 \
		--privileged \
		--security-opt seccomp=unconfined \
		$(DISPLAY_ARGS) \
		$(JAVA_ARGS) \
		$(X11_ARGS) \
		-v "$(PWD):/project" \
		$(IMAGE) || true
	$(XHOST_RESET_CMD)

shell: run

sdimage:
	$(PROJECT_ROOT)/tools/build_sdimage.sh

clean:
	-docker rm -f $(CONTAINER)
	-docker image rm $(IMAGE)
