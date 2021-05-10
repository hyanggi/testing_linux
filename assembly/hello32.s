.global _start          # export symbol "_start" for the linker (ld) to read

.text                   # text section
_start:                 # label "_start"
    movl $4, %eax       # system call number (sys_write)
    movl $1, %ebx       # file descriptor (stdout)
    movl $msg, %ecx     # address of the char string to write
    movl $len, %edx     # length of the string
    int  $0x80          # system call

    movl $1, %eax       # system call number (sys_exit)
    movl $0, %ebx       # exit status
    int  $0x80          # system call

.data                   # data section
msg:                    # label "msg"
    .ascii "Hello, world! (32)\n"
    len = . - msg       # symbol "len" set to the current address (.) subtracted by "msg"
