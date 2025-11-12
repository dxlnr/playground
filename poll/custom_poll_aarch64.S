    .text
    .global custom_poll
    .type   custom_poll, %function

custom_poll:
    // __NR_ppoll = 73 on Linux/aarch64
    mov     x8, #73
    svc     #0
    // return (x0 already has return value)
    ret

    .size   custom_poll, .-custom_poll
