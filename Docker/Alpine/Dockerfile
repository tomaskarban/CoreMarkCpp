# First stage - build CoreMarkCpp
FROM alpine:latest AS builder
RUN apk update && apk upgrade && apk add build-base cmake gcc git ninja
RUN git clone https://github.com/tomaskarban/CoreMarkCpp.git
WORKDIR /CoreMarkCpp
RUN cmake -B build -D CMAKE_BUILD_TYPE=Release -G Ninja
RUN ninja -C build

# Second stage - create a minimal image
FROM alpine:latest
COPY --from=builder /CoreMarkCpp/build/CoreMarkCpp /usr/local/bin/
CMD ["CoreMarkCpp"]
