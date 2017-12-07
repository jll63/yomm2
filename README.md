# Don't...fork...yet...

Don't clone or fork this yet! I may and I will (eventually) change history! But
if you do it anyway I'm fine with it, just don't bark at me later!

I am working on the next iteration of my open method library. While porting it
to D I came up with some fresh ideas. Some (but not all) of them can be ported
back to C++. Maybe.

If things go well yomm2 - the successor of yomm11 - will improve on yomm11 in
the following ways:

- Entry point in an open method will be an ordinary function, not a `constexpr`
  function object
- ...thus it will be possible to overload open methods
- ...and form pointers to open methods
- Overriding methods across namespaces was a hassle. It shoud be easier.
- We had to choose between a fast intrusive mode and a slow orthogonal mode. I
  think I can support a fast orthgonal mode. See my article on the D blog.
