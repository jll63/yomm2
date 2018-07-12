call_approve(Role const&, Expense const&, double):     # @call_approve(Role const&, Expense const&, double)
	.cfi_startproc
# %bb.0:
	movq	dispatch_data::instance<void>::_+24(%rip), %rax
	movb	dispatch_data::instance<void>::_+32(%rip), %cl
	movq	(%rdi), %rdx
	movq	-8(%rdx), %rdx
	imulq	%rax, %rdx
	shrq	%cl, %rdx
	movq	dispatch_data::instance<void>::_(%rip), %r8
	movq	method<approve>::slots_strides(%rip), %r9
	movq	(%r8,%rdx,8), %r10
	movq	(%rsi), %rdx
	imulq	-8(%rdx), %rax
	shrq	%cl, %rax
	movslq	(%r9), %rcx
	movq	(%r10,%rcx,8), %rcx
	movq	(%r8,%rax,8), %rax
	movslq	8(%r9), %rdx
	movslq	(%rax,%rdx,8), %rax
	movslq	16(%r9), %rdx
	imulq	%rax, %rdx
	movq	(%rcx,%rdx,8), %rax
	jmpq	*%rax                   # TAILCALL
