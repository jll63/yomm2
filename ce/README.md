# YOMM2 on Compiler Explorer

The following examples compare open methods with the equivalent (closed)
virtual function based approaches.

| OOP                   |     | YOMM2                                 | CE link                         |
| --------------------- | --- | ------------------------------------- | ------------------------------- |
| virtual function call | vs  | uni-method call via plain reference   | https://godbolt.org/z/exv73b9db |
| virtual function call | vs  | uni-method call via virtual_ptr       | https://godbolt.org/z/zKMqcMfx8 |
| double dispatch       | vs  | multi-method call via plain reference | https://godbolt.org/z/a1j3Exhbo |
| double dispatch       | vs  | multi-method call via virtual_ptr     | https://godbolt.org/z/PT9P7q6M6 |

YOMM2 can also [add polymorphic operations to non-polymorphic
classes](https://godbolt.org/z/Tsofsejb7).