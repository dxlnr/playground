# BUILD
aarch64-linux-gnu-gcc -Wall -O2 server.c -o server
# RUN
qemu-aarch64 -L /usr/aarch64-linux-gnu ./server
