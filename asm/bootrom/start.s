.global _Reset
_Reset:
  ldr sp, =stack_top
  bl c_entry
  b .
