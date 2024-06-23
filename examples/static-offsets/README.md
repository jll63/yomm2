
virtual function call
	mov	rax, qword ptr [rdi]
	jmp	qword ptr [rax + 16]            # TAILCALL


method, via virtual_ptr, dynamic offsets
	mov	rax, qword ptr [rip + _ZN5yorel5yomm26methodI12YoMm2_S_kickFvNS0_11virtual_ptrI6AnimalNS0_6policy7releaseEEEES6_E2fnE]
	mov	rax, qword ptr [rsi + 8*rax]
	jmp	rax                             # TAILCALL

method, via virtual_ptr, static offsets
	mov	rax, qword ptr [rsi + 16]
	jmp	rax                             # TAILCALL

2-method, virtual_ptr, dynamic offsets
	mov	rax, qword ptr [rip + _ZN5yorel5yomm26methodI12YoMm2_S_meetFvNS0_11virtual_ptrI6AnimalNS0_6policy7releaseEEES7_ES6_E2fnE]
	mov	rax, qword ptr [rsi + 8*rax]
	mov	r8, qword ptr [rip + _ZN5yorel5yomm26methodI12YoMm2_S_meetFvNS0_11virtual_ptrI6AnimalNS0_6policy7releaseEEES7_ES6_E2fnE+8]
	mov	r8, qword ptr [rcx + 8*r8]
	imul	r8, qword ptr [rip + _ZN5yorel5yomm26methodI12YoMm2_S_meetFvNS0_11virtual_ptrI6AnimalNS0_6policy7releaseEEES7_ES6_E2fnE+16]
	mov	rax, qword ptr [rax + 8*r8]
	jmp	rax                             # TAILCALL


2-method, virtual_ptr, static offsets
	mov	rax, qword ptr [rsi]
	mov	r8, qword ptr [rcx + 8]
	lea	r8, [r8 + 2*r8]
	mov	rax, qword ptr [rax + 8*r8]
	jmp	rax                             # TAILCALL
