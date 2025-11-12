    .text
    .global custom_poll
    .type   custom_poll, %function

custom_poll:
    // Reserve space for struct timespec on stack (16 bytes)
    sub     sp, sp, #16
    // seconds = timeout_ms / 1000
    mov     x5, x2          // x5 = timeout_ms (zero-extended)
    mov     x6, #1000
    udiv    x7, x5, x6      // x7 = seconds
    // remainder_ms = timeout_ms - seconds * 1000
    msub    x8, x7, x6, x5  // x8 = timeout_ms - x7*1000
    // nanoseconds = remainder_ms * 1,000,000
    // 1,000,000 = 0x00000000000F4240
    movz    x9, #0x4240          // low 16 bits
    movk    x9, #0x000F, lsl #16 // upper bits: 0xF << 16
    mul     x8, x8, x9
    // store timespec { tv_sec, tv_nsec } on stack
    str     x7, [sp]        // tv_sec
    str     x8, [sp, #8]    // tv_nsec
    // Set up ppoll args:
    // x0 = fds              (already)
    // x1 = nfds             (already)
    mov     x2, sp          // tmo_p = &timespec
    mov     x3, xzr         // sigmask = NULL
    mov     x4, xzr         // sigsetsize = 0
    // __NR_ppoll = 73 on Linux/aarch64
    mov     x8, #73
    svc     #0
    // restore stack
    add     sp, sp, #16
    // return (x0 already has return value)
    ret

    .size   custom_poll, .-custom_poll
