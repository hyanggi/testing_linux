.global _start          # export symbol "_start" for the linker (ld) to read

.text                   # text section
_start:                 # label "_start"
    mov $1, %rax        # system call number (sys_write)
    mov $1, %rdi        # file descriptor (stdout)
    mov $msg, %rsi      # address of the char string to write
    mov $len, %rdx      # length of the string
    syscall             # system call

    mov $60, %rax       # system call number (sys_exit)
    mov $0, %rdi        # exit status
    syscall             # system call

.data                   # data section
msg:                    # label "msg"
    .ascii "Hello, world! (64)\n"
    len = . - msg       # symbol "len" set to the current address (.) subtracted by "msg"
