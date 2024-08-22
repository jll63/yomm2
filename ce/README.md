# YOMM2 on Compiler Explorer

YOMM2 is available on Compiler Explorer. Make sure that you also select Boost
version 1.74 or above, and you probably want to add the `-O3 -DNDEBUG` compiler
switches.

The following examples are available:

* The [examples](https://jll63.github.io/yomm2/ce/slides.html) from the slides.
* The matrix example from the GitHub langing page.

The following examples use the diff mode to compare open methods with the
equivalent (closed) virtual function based approaches.

* [virtual function call vs uni-method call via plain reference](https://jll63.github.io/yomm2/ce/vf-vs-1m-ref.html)
* [virtual function call vs uni-method call via virtual_ptr  ](https://jll63.github.io/yomm2/ce/vf-vs-1m-vptr.html)
* [double dispatch    vs multi-method call via plain reference](https://jll63.github.io/yomm2/ce/2d-vs-2m-ref.html)
* [double dispatch    vs multi-method call via virtual_ptr ](https://jll63.github.io/yomm2/ce/2d-vs-2m-vptr.html)

YOMM2 can also [add polymorphic operations to non-polymorphic
classes](https://jll63.github.io/yomm2/ce/vptr-final.html).

When `virtual_ptr` is used in combination with generated static offsets, method
dispatch matches the speed of virtual functions. It is also possible to generate
dispatch data that can be installed without calling `update`, a fairly expensive
operaiton. See [this example](https://jll63.github.io/yomm2/ce/generator.html).
