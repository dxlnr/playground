# BUILD
aarch64-linux-gnu-gcc -Wall -O2 -DCUSTOM server.c custom_poll_aarch64.S -o server
# RUN
qemu-aarch64 -L /usr/aarch64-linux-gnu ./server
