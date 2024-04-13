
<span style="font-size:xx-large;"><strong>declare_method</strong><br/></span><br/>
<sub>defined in <yorel/yomm2/cute.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

```c++
#define declare_method(return-type, method, (types)) /*unspecified*/
```

### Usage

```
declare_method(return-type, method, (types)) {
    ...
}
```

Declare a method.

Create an inline function `method` that returns `return-type` and takes a
parameter list consisting of `types`. At least one of `types` (but not
necessarily all) must be a *virtual parameter*, i.e. in the form
[`virtual_<T>`](virtual_.md), or [`virtual_ptr<T>`](virtual_ptr.md). The
`virtual_` decorator is stripped from `types`.

When `method` is called, the dynamic types of the virtual arguments are
examined, and the most specific definition compatible with `unspecified_type...`
is called. If no compatible definition exists, or if several compatible
definitions exist but none of them is more specific than all the others, the
call is illegal and an error handler is executed. By default it writes a
diagnostic on `std::cerr ` and terminates the program via `abort`. The handler
can be customized.

NOTE:

* The method parameter list _must_ be surrounded by parentheses.

* The parameters in `types` consist of _just_ a type, e.g. `int` is correct
  but `int i` is not.

* Types that contain commas (e.g. `tuple<int, char>`) cannot be used directly as
  macro arguments. The workarounds are:
  * using an alias, e.g. `using int_char = tuple<int, char>`
  * using any C++ construct that puts parentheses around the commas,
    e.g. `decltype(std::tuple<int, int>())`
  * [`BOOST_IDENTITY_TYPE`](https://www.boost.org/doc/libs/1_82_0/libs/utility/identity_type/doc/html/index.html)

## synonym
YOMM2_DECLARE, defined in <yorel/yomm2/macros.hpp>, also provided by <yorel/yomm2.hpp>.

## Example
```
declare_method(std::string, kick, (virtual_<Animal&>));
declare_method(std::string, meet, (virtual_<Animal&>, virtual_<Animal&>));
declare_method(bool, approve, (virtual_<Role&>, virtual_<Expense&>), double);
```
