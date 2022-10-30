.text
.global add
.type global, @function

add:
	movq $0, %rax
	addq %rdi, %rax
	addq %rsi, %rax
	ret
