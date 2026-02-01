
# YOMM2

YOMM2 was a library that implemented fast, open, multi-methods for C++17. It is
was inspired by the papers by Peter Pirkelbauer, Yuriy Solodkyy, and Bjarne
Stroustrup. The original README is [here](ORIGINAL-README.md).

YOMM2 has been superseded by
[Boost.OpenMethod](https://www.boost.org/doc/libs/latest/libs/openmethod/doc/html/openmethod/index.html) - an official Boost library.

Porting from YOMM2 to OpenMethod is easy - see below a copy of my announcement
on reddit:

## YOMM2 is reborn as Boost.OpenMethod

In early 2025, I submitted YOMM2 for inclusion in the Boost libraries, under the
name OpenMethod. The library underwent formal review, and it was accepted with
conditions. OpenMethod became part of Boost in version 1.90.

As a consequence, I am discontinuing work on YOMM2.

[Boost.OpenMethod](https://www.boost.org/doc/libs/latest/libs/openmethod/doc/html/openmethod/index.html) is available to download from:

- the [Boost website](https://www.boost.org)
- [vcpkg](https://vcpkg.roundtrip.dev/ports/boost-openmethod) as a modular
  package with dependencies to the required Boost libraries
- [Conan](https://conan.io/center/recipes/boost) as part of the whole Boost
  package

OpenMethod is available on [Compiler Explorer](https://godbolt.org/z/W6684Tfaz) - make sure to select Boost 1.90 (or above) in Libraries.

I encourage YOMM2 users to switch to OpenMethod as quickly as convenient.
OpenMethod is not directly backward compatible with YOMM2. However, migrating
from one to the other should be painless for all basic uses of the library - see
an example at the end of this post. If you used advanced features such as
policies and facets, a little more work may be required, but the underlying
ideas remain the same, yet presented in a more ergonomic way.

## What Has Changed and Why?

On the surface, a lot has changed, but, just underneath, it is the same library,
only better. Much better, in my (biased) opinion. This is due to:

- The freedom to clean up and rework a library that has evolved over seven
  years, without being bound by backward compatibility.

- The feedback - comments, suggestions, criticisms - gathered during the Boost
  formal review.

I will go through the major changes in this section, starting with the most
basic features, then going into more advanced ones.

There was a lot of renaming, obviously. `yorel::yomm2` is now
`boost::openmethod`. `setup` becomes `initialize`. Method specializations are
now called "overriders".

`declare_method` and `define_method` become `BOOST_OPENMETHOD` and
`BOOST_OPENMETHOD_OVERRIDE`, and the return type moves from first macro
parameter to third - i.e., just after the method's parameter list. This is not
gratuitous, nor an attempt at "looking modern". This solves a minor irritation:
return types can now contain commas, without the need for workarounds such as
using a typedef or `BOOST_IDENTITY_TYPE`.

`virtual_ptr` was an afterthought in YOMM2. In OpenMethod, it becomes the
preferred way of passing virtual arguments. It also now supports all the
operations normally expected on a smart pointer. `virtual_` still exists, but it
is dedicated to more advanced use cases like embedding a vptr in an object.

No names (excepting macros) are injected in the global namespace anymore. The most frequently used constructs can be imported in the current namespace with `using namespace boost::openmethod::aliases`.

Constructs that were previously undocumented have been cleaned up and made
public. The most important is `virtual_traits`, which governs what can be used
as a virtual parameter, how to extract the polymorphic part of an argument (e.g.
a plain reference, a smart pointer to an object, ...), how to cast virtual
arguments to the types expected by the overriders, etc. This makes it possible
to use your favorite smart pointer in virtual parameters.

"Policies" and "facets" are now called "registries" and "policies". That part of
YOMM2 relied heavily on the CRTP. Policies (ex-facets) are now MP11-style quoted
metafunctions that take a registry. So, CRTP is still there, but it is not an
eyesore anymore. The policies/facets that were used only in setup/initialize
(like tracing the construction of dispatch data) are now optional arguments of
`initialize`.

The most recent experiment in YOMM2 revolved around moving stuff to compile
time: method dispatch tables (for reduced footprint); and method offsets in the
v-tables (for scraping the last bit of performance). It did not make it into
OpenMethod. I have not lost interest in the feature though. It will reappear at
some point in the future, hopefully in a more convenient manner.

## Porting from YOMM2 to OpenMethod

Many of the examples can be ported in a quick-and-dirty manner using a
compatibility header such as:

```c++
// <yomm2_to_bom.hpp>

#include <boost/openmethod.hpp>
#include <boost/openmethod/initialize.hpp>

#define register_classes BOOST_OPENMETHOD_CLASSES
#define declare_method(RETURN, ID, ARGS) \
    BOOST_OPENMETHOD(ID, ARGS, RETURN)
#define define_method(RETURN, ID, ARGS) \
    BOOST_OPENMETHOD_OVERRIDE(ID, ARGS, RETURN)
using boost::openmethod::virtual_;

namespace yorel {
    namespace yomm2 {
        void update() {
            boost::openmethod::initialize();
        }
    }
}
```

For example, here is the "adventure" example [on Compiler
Explorer](https://godbolt.org/z/1G5jjzzfz) using the compatibility header.

A proper port takes little more effort:

1. Move the return types using a simple regex substitution.
2. Change the initialization (typically only in `main`'s translation unit).
3. Switch virtual arguments from `virtual_` to `virtual_ptr` (not mandatory but
   highly recommended).

Here is "adventure", fully ported to Boost.OpenMethod, [on Compiler
Explorer](https://godbolt.org/z/5TjxM85Ej).

## Support

Support is available on a purely voluntary, non-committal basis from the author.
The Boost community as a whole has a good record of welcoming requests and
suggestions from their users. Please reach out to:

- The [Boost Users mailing
  list](https://lists.boost.org/mailman3/lists/boost-users.lists.boost.org/) -
  please put [openmethod] at the beginning of the subject
- The [#boost-openmethod
  channel](https://app.slack.com/client/T21Q22G66/C0A16HR702G?selected_team_id=T21Q22G66)
  on the CppLang Slack
