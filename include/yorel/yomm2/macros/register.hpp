#ifndef YOMM2_GENSYM

#include<yorel/yomm2/macros/gensym.hpp>

#define YOMM2_REGISTER(...) static __VA_ARGS__ YOMM2_GENSYM

#endif
