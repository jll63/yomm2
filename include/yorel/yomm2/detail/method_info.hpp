namespace yorel {
namespace yomm2 {
namespace detail {

struct definition_info;

struct yOMM2_API method_info : static_chain<method_info>::static_link {
    std::string_view name;
    type_id *vp_begin, *vp_end;
    static_chain<definition_info> specs;
    void* ambiguous;
    void* not_implemented;
    type_id method_type;
    size_t* slots_strides_ptr;

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

} // namespace detail
} // namespace yomm2
} // namespace yorel
