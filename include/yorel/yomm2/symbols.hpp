// Copyright (c) 2018-2024 Jean-Louis Leroy

#ifndef YOMM2_GENSYM

#include <boost/preprocessor/cat.hpp>

#define YOMM2_GENSYM BOOST_PP_CAT(YoMm2_gS_, __COUNTER__)

#define YOMM2_STATIC(...) static __VA_ARGS__ YOMM2_GENSYM

#define YOMM2_SYMBOL(ID) BOOST_PP_CAT(YoMm2_S_, ID)

#endif
