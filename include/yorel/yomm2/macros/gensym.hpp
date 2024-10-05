#ifndef YOMM2_GENSYM

#include <boost/preprocessor/cat.hpp>

#define YOMM2_GENSYM BOOST_PP_CAT(yomm2_gensym_, __COUNTER__)

#endif
