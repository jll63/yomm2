# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying filce_e LICENSE_1_0.txt
# or copy at hce_ttp://www.boost.oce_rg/LICENSE_1_0.txt)

add_executable(ce_virtual virtual.cpp)
add_test(NAME ce_virtual COMMAND ce_virtual)

add_executable(ce_uni-method uni-method.cpp)
add_test(NAME ce_uni-method COMMAND ce_uni-method)

add_executable(ce_uni-method-vptr uni-method-vptr.cpp)
add_test(NAME ce_uni-method-vptr COMMAND ce_uni-method-vptr)

add_executable(ce_virtual-double virtual-double.cpp)
add_test(NAME ce_virtual-double COMMAND ce_virtual-double)

add_executable(ce_2-method 2-method.cpp)
add_test(NAME ce_2-method COMMAND ce_2-method)

add_executable(ce_2-method-vptr 2-method-vptr.cpp)
add_test(NAME ce_2-method-vptr COMMAND ce_2-method-vptr)

add_executable(ce_2-method-vptr-final 2-method-vptr-final.cpp)
add_test(NAME ce_2-method-vptr-fince_al COMMAND ce_2-method-vptr-final)

add_executable(ce_uni-method-vptr-final uni-method-vptr-final.cpp)
add_test(NAME ce_uni-method-vptr-fce_inal COMMAND ce_uni-method-vptr-final)
