# CoreMarkCpp as a Docker Container

CoreMarkCpp is a small executable that detects the number of available CPU cores at runtime.
We use the classic approach of a two-stage Docker container build, using the official
Alpine Docker image.

The first stage

 - updates the base image,
 - installs git, cmake, ninja, and GCC,
 - clones CoreMarkCpp source code from https://github.com/tomaskarban/CoreMarkCpp,
 - and builds a release binary.

The second stage starts with a clean base image and copies the binary we built
in the first stage. The binary is linked statically, so it has no other library dependencies.
Its size on x64 architecture is only about 180 kB.

## Container Contents

The container is based on Alpine official Docker image.
We only add one self-contained binary and execute it when the container starts.

## Building and Running the Container

Simple and straightforward:

```
docker build -t coremarkcpp .
docker run --name coremarkcpp coremarkcpp
```

The container stops at the end. You can run the benchmark again:

```
docker start -ai coremarkcpp
```
