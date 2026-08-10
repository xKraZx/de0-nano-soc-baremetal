
## Overview

This repository provides a bare-metal development environment for the **Terasic DE0-Nano-SoC** board, including the FPGA hardware design, bare-metal software, and tools required to build a bootable SD card image.

For convenience, a **Docker-based development environment** is also provided. Docker is optional and can be used to set up an isolated and reproducible development environment, including access to the **Quartus GUI** when needed.

## Requirements

There are two ways to use this project:

* **Docker** – recommended for a convenient and isolated development environment.
* **Native environment** – use the project directly on the host system without Docker. In this case, all required dependencies must be installed manually.

---

## Using Docker

Docker provides all project-specific dependencies inside the container, so no additional development packages need to be installed on the host.

### Host Requirements

The following tools are required:

* **Linux**
* **Docker Engine**
* **Docker Buildx**
* **GNU Make**
* **X11** and the `xhost` utility
* Access to the `/dev/loop0` device
* A valid `DISPLAY` environment variable for the Quartus GUI

You can verify the required tools with:

```bash
docker --version
docker buildx version
make --version
xhost --version
```

### Build and Run

From the project root directory, build the Docker image:

```bash
make build
```

Then start the container:

```bash
make run
```

The project directory is mounted inside the container as:

```text
/project
```

Once inside the container, the project can be built using:

```bash
cd /project
./build_sd
```

The **Quartus GUI** is also available from inside the container. The container is configured to forward the host's X11 display, allowing graphical applications to be displayed directly on the host.

---

## Using the Native Environment

Docker is not required to use the project.

If you prefer to work directly on the host system, you must install all required dependencies manually, including **Intel Quartus** and the tools required by the hardware and software build systems.

The `build_sd` script can then be used to build the complete project and generate the SD card image:

```bash
./build_sd
```

Before running the script, make sure that the required environment variables and tools are available. In particular, `QUARTUS_ROOTDIR` must point to the Quartus installation directory.

This approach does not use the Docker configuration or the `Makefile` targets for building and running the development environment.
