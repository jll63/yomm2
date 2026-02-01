> [!IMPORTANT]
> YOMM2 has been superseded by Boost.OpenMethod. See README for details.


<span style="font-size:xx-large;"><strong>RestrictedOutputStream</strong><br/></span><br/>

    Stream& operator<<(Stream& os, const std::string_view& view);
    Stream& operator<<(Stream& os, const void* value);
    Stream& operator<<(Stream& os, size_t value);

(where `Stream` is the type of the `error_stream` data member)
