# Without error code
```
Address         Content
rsp+8           rax
rsp+16          rbx
rsp+24          rcx
rsp+32          rdx
rsp+40          rsi
rsp+48          rdi
rsp+56          rbp
rsp+64          r8
rsp+72          r9
rsp+80          r10
rsp+88          r11
rsp+96          r12
rsp+104         r13
rsp+112         r14
rsp+120         r15
rsp+128         interrupt number
rsp+136         rip
rsp+144         cs
rsp+152         rflags
rsp+160         rsp
rsp+168         ss
```

In case of new thread `rip` should point to `switch_task_int_return` and the stack top should be entry point to new thread's main.