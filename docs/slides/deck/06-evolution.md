# Evolution of YOMM2

<!-- .slide: class="title"  -->
<!-- .slide: class="center" -->



## Past

<br/>

<ul>
  <li class="fragment">goals:
    <ul>
      <li class="fragment">help promote adoption in the standard
        <ul>
          <li class="fragment">submit to Boost</li>
          <li class="fragment">talk about it (CppCon 2018...)</li>
        </ul>
      </li>
    </ul>
  </li>
  <li class="fragment">results:
    <ul>
      <li class="fragment">Boost community: not interested</li>
      <li class="fragment">standard committee: not interested</li>
      <li class="fragment">2018-2020: bug fixes, cleanup...</li>
    </ul>
  </li>
</ul>




## Present

<br/>

<ul>
  <li class="fragment">2020: gave up on adoption in the standard</li>
  <li class="fragment">new developments
    <ul>
      <li class="fragment">virtual_ptr</li>
      <li class="fragment">header only (+ Compiler Explorer)</li>
      <li class="fragment">core API</li>
      <li class="fragment">interoperate with templates</li>
      <li class="fragment">member methods</li>
      <li class="fragment">policies and facets
        <ul>
          <li class="fragment">custom RTTI</li>
          <li class="fragment">custom error handling, trace, vptr placement...</li>
        </ul>
      </li>
    </ul>
  </li>
</ul>




## virtual_ptr

<br/>

```c++
declare_method(int, value, (virtual_ptr<Node>));
```

```asm
mov	rax, qword ptr [rip + method<value, int (virtual_ptr<Node>)::fn+80]
mov	rax, qword ptr [rsi + 8*rax]
jmp	rax
```




## virtual_ptr

```c++
auto make_node_ptr(Node& node, virtual_ptr<Node>& p) {
    return virtual_ptr<Node>(node);
}
```

```asm
	mov	rax, rdi
	mov	rcx, qword ptr [rdi]
	mov	rdx, qword ptr [rcx - 8]
	lea	rcx, [rip + typeinfo for Node]
	cmp	rdx, rcx
	je	.LBB7_1
	imul	rdx, qword ptr [rip + fast_perfect_hash<release>::hash_mult]
	movzx	ecx, byte ptr [rip + fast_perfect_hash<release>::hash_shift]
	shr	rdx, cl
	shl	rdx, 3
	add	rdx, qword ptr [rip + vptr_vector<release>::vptrs]
	mov	rdx, qword ptr [rdx]
	ret
.LBB7_1:
	lea	rdx, [rip + method_tables<release>::static_vptr<Node>]
	mov	rdx, qword ptr [rdx]
	ret
```




## virtual_ptr

```c++
auto make_final_node_ptr(Node& node, virtual_ptr<Node>& p) {
    return final_virtual_ptr(node);
}
```

classes need not be polymorphic

<br/>

```asm
mov	rax, rdi
mov	rdx, qword ptr [rip + method_tables<release>::static_vptr<Node>]
ret
```



## Future

<br/>

<ul>
  <li class="fragment">goals:
    <ul>
      <li class="fragment">match (maybe beat) virtual function speed</li>
      <li class="fragment">pre-calculate dispatch tables</li>
      <li class="fragment">malloc-free operation</li>
      <li class="fragment">C++20</li>
    </ul>
  </li>
</ul>
