
yorel::yomm2::policy::deferred_static_rtti

    struct deferred_static_rtti : virtual rtti {};

The `deferred_static_rtti` facet, derived from `rtti`, directs YOMM2 to defer
collection of static type information until `update` is called. This makes it
possible to interface with custom RTTI systems that use static constructors to
assign type information.

## See also

The custom RTTI [tutorial](/yomm2/tutorials/custom_rtti_tutorial.html).
