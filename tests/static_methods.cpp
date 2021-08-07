// Copyright (c) 2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/cute.hpp>

#include <string>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using yorel::yomm2::virtual_;
using std::cout;

class Engineer {
  public:
    virtual ~Engineer() {}
};

class SeniorEngineer : public Engineer {};

register_class(Engineer);
register_class(SeniorEngineer, Engineer);

declare_method(std::string, speak, (virtual_<const Engineer&>));

define_method(std::string, speak, (const Engineer&)) {
  return "engineers love ADL";
}

define_method(std::string, speak, (const SeniorEngineer& engineer)) {
  return "senior " + next(engineer);
}

struct no_adl {
    declare_static_method(std::string, speak, (virtual_<const Engineer&>));
};

define_method(std::string, no_adl::speak, (const Engineer&)) {
  return "engineers hate ADL";
}

define_method(std::string, no_adl::speak, (const SeniorEngineer& engineer)) {
  return "senior " + next(engineer);
}

BOOST_AUTO_TEST_CASE(adl) {
    yorel::yomm2::update_methods();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(speak(engineer) == "engineers love ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(speak(engineer) == "senior engineers love ADL");
    }
}

BOOST_AUTO_TEST_CASE(noadl) {
    yorel::yomm2::update_methods();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(no_adl::speak(engineer) == "engineers hate ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(no_adl::speak(engineer) == "senior engineers hate ADL");
    }
}
