// dl_main.cpp
// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* compile with the following commands:
   g++ -std=c++11 -I../include -c dl_main.cpp
   g++ -std=c++11 -I../include -c ../src/multi_methods.cpp
   g++ dl_main.o multi_methods.o -o dl_main -ldl -rdynamic
   g++ -std=c++11 -I../include -fPIC -c -o dl_shared.o dl_shared.cpp
   g++ -shared -Wl,-soname,dl_shared.so -o dl_shared.so dl_shared.o
*/

#include <iostream>
#include <dlfcn.h>

#include <yorel/yomm2.hpp>

#include "dl.hpp"

using namespace std;
using yorel::yomm2::virtual_;

YOMM2_DEFINE(string, encounter, (const Animal&, const Animal&)) {
  return "ignore";
}

YOMM2_DEFINE(string, encounter, (const Herbivore&, const Carnivore&)) {
  return "run";
}

int main() {
  yorel::yomm2::update_methods();

  cout << "Before loading library\n";
  cout << "encounter(Cow(), Wolf()) -> " << encounter(Cow(), Wolf()) << endl;
  cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

  void* handle = dlopen(
#ifdef __APPLE__
      "libdl_shared.dylib"
#else
      "libdl_shared.so"
#endif
      , RTLD_NOW);

  if (!handle) {
    cout << "dlopen() failed: " << dlerror() << "\n";
    exit(1);
  }

  cout << "\nAfter loading library\n";
  yorel::yomm2::update_methods();

  using make_tyget_type = Animal* (*)();
  make_tyget_type make_tiger = reinterpret_cast<make_tyget_type>(dlsym(handle, "make_tiger"));

  if (!make_tiger) {
    cout << "dlsym() failed: " << dlerror() << "\n";
    exit(1);
  }

  cout << "encounter(Cow(), *make_tiger()) -> " << encounter(Cow(), *make_tiger()) << endl;
  cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

  dlclose(handle);

  cout << "\nAfter unloading library\n";
  yorel::yomm2::update_methods();

  cout << "encounter(Cow(), Wolf()) -> " << encounter(Cow(), Wolf()) << endl;
  cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

  return 0;
}
