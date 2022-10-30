.data

msg: .ascii "Hello World!\n"
len = . - msg

.text
.global hello_world
.type hello_world, @function

hello_world:
	pushq %rbp
	movq %rsp, %rbp
	movq $1, %rax
	movq $1, %rdi
	leaq msg(%rip), %rsi
	movq $len, %rdx
	syscall
	popq %rbp
	ret
