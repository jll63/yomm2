// dl_main.cpp
// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include <dlfcn.h>
#include <iostream>

#include <yorel/yomm2/keywords.hpp>

#include "dl.hpp"

using namespace std;

define_method(string, encounter, (const Animal&, const Animal&)) {
    return "ignore";
}

int main() {
    yorel::yomm2::update_methods();

    cout << "Before loading library\n";
    cout << "encounter(Cow(), Wolf()) -> " << encounter(Cow(), Wolf()) << endl;
    cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

    char dl_path[4096];
    dl_path[readlink("/proc/self/exe", dl_path, sizeof(dl_path))] = 0;
    *strrchr(dl_path, '/') = 0;

#ifdef __APPLE__
    strcat(dl_path, "/libdl_shared.dylib");
#else
    strcat(dl_path, "/libdl_shared.so");
#endif

    void* handle = dlopen(dl_path, RTLD_NOW);

    if (!handle) {
        cout << "dlopen() failed: " << dlerror() << "\n";
        exit(1);
    }

    cout << "\nAfter loading library\n";
    yorel::yomm2::update_methods();

    using make_tyget_type = Animal* (*)();
    make_tyget_type make_tiger =
        reinterpret_cast<make_tyget_type>(dlsym(handle, "make_tiger"));

    if (!make_tiger) {
        cout << "dlsym() failed: " << dlerror() << "\n";
        exit(1);
    }

    cout << "encounter(Cow(), *make_tiger()) -> "
         << encounter(Cow(), *make_tiger()) << endl;
    cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

    dlclose(handle);

    cout << "\nAfter unloading library\n";
    yorel::yomm2::update_methods();

    cout << "encounter(Cow(), Wolf()) -> " << encounter(Cow(), Wolf()) << endl;
    cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

    return 0;
}
