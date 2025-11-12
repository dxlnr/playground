# BUILD
aarch64-linux-gnu-gcc -Wall -O2 -DCUSTOM server.c custom_poll.S -o server
# RUN
qemu-aarch64 -L /usr/aarch64-linux-gnu ./server
