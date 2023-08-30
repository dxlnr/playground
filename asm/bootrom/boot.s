.global _start

_start:
    ldr r0, =0x400E0800
    ldr r1, =message

loop:
    ldrb r2, [r1], #1
    cmp r2, #0
    beq done

    strb r2, [r0, #0x20]
    b loop

done:
    b done

message:
    .asciz "Hello!\n"
