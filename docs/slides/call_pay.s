
call_pay_vfunc(Employee const&):
	movq	(%rdi), %rax
	jmp	*16(%rax)

call_pay_method(Employee const&):

;;; clang 5.0
   	movq	dispatch_data(%rip), %rax              ; hash table
	movb	dispatch_data+32(%rip), %cl            ; shift factor
	movslq	method<pay>::slots_strides(%rip), %rdx ; slot
	movq	(%rdi), %rsi                           ; vptr
	movq	-8(%rsi), %rsi                         ; type_info ptr
	imulq	dispatch_data+24(%rip), %rsi           ; hash: multiply
	shrq	%cl, %rsi                              ; hash: shift
	movq	(%rax,%rsi,8), %rax                    ; method table
	jmpq	*(%rax,%rdx,8)          # TAILCALL     ; call function

;;; gdc 6.0
 	movq	(%rdi), %rax                           ; vptr
	movq	dispatch_data+32(%rip), %rcx           ; shift factor
	movslq	method<pay>::slots_strides(%rip), %rdx ; slot
	movq	-8(%rax), %rax                         ; type_info ptr
	imulq	dispatch_data+24(%rip), %rax           ; hash: multiply
	shrq	%cl, %rax                              ; hash: shift
	movq	dispatch_data(%rip), %rcx              ; hash table
	movq	(%rcx,%rax,8), %rax                    ; method table
	jmp	*(%rax,%rdx,8)                             ; call function
