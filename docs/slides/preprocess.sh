clang++-5.0 -std=c++17 -I ~/dev/yomm2/include -E rolex.cpp \
    | clang-format \
    | perl -pe '
s/::yorel::yomm2::(detail::)?//g;
s/virtual_arg_t<virtual_<([^>]+)>>/$1/g;
s/    /  /g;
'
