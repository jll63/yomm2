> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>deferred_static_rtti</strong></span><br/>

```c++
struct deferred_static_rtti : virtual rtti {};
```

The `deferred_static_rtti` facet, derived from `rtti`, directs YOMM2 to defer
collection of static type information until `update` is called. This makes it
possible to interface with custom RTTI systems that use static constructors to
assign type information.

## See also

The custom RTTI [tutorial](/yomm2/tutorials/custom_rtti_tutorial.html).
