#ifndef YOREL_OPENMETHODS_INCLUDED
#define YOREL_OPENMETHODS_INCLUDED

#include <vector>

#ifndef NDEBUG
#define YOMM2_DEBUG(X) X
#define YOMM2_COMMA_DEBUG(X) , X
#else
#define YOMM2_DEBUG(ST)
#define YOMM2_COMMA_DEBUG(X)
#endif

namespace yorel {
namespace openmethods {

template<typename T>
struct virtual_;

template<typename T>
struct virtual_<T*> {
    virtual_(T* obj) : _obj(obj) {}
    T* ptr() const { return _obj; }
    T* _obj;
};

template<typename T>
struct virtual_<T&> {
    virtual_(T& obj) : _obj(&obj) {}
    T* ptr() const { return _obj; }
    T* _obj;
};

template<typename Key, typename... Args>
struct Method;

namespace details {

struct resolve {};

template<typename Spec, typename Method YOMM2_COMMA_DEBUG(const char Name[])>
struct Register_spec
{
    YOMM2_DEBUG(Register_spec() {
            std::cout << "Register " << Name << "\n";
        })
    static Register_spec instance;
    static constexpr const char* name() { return Name; }
};

template<typename Spec, typename Method YOMM2_COMMA_DEBUG(const char Name[])>
Register_spec<Spec, Method YOMM2_COMMA_DEBUG(Name)> Register_spec<Spec, Method YOMM2_COMMA_DEBUG(Name)>::instance;

} // namespace details

} // namespace openmethods
} // namespace yorel

#define YOMM2_ID(Id) _yorel_openmethods_ ## Id

//#define YOMM2_METHOD_DESC()

#define declare_method(ReturnType, Fun, ...)                                  \
    struct YOMM2_ID(Fun);                                                     \
    ::yorel::openmethods::Method< YOMM2_ID(Fun), __VA_ARGS__ >                \
    Fun(::yorel::openmethods::details::resolve, __VA_ARGS__);                 \
    template<typename Signature> struct YOMM2_ID(Fun ## _spec);               \
    ReturnType Fun(__VA_ARGS__) { std::cout << #Fun << '(' << #__VA_ARGS__ << ")\n"; }

#define begin_method(ReturnType, Fun, ...)                                    \
    template<> struct YOMM2_ID(Fun ## _spec)<void(__VA_ARGS__)> {             \
        template<typename Signature> struct resolved;                         \
        template<typename... Args> struct resolved<void(Args...)> {           \
            using type =                                                      \
                decltype(Fun(::yorel::openmethods::details::resolve(),        \
                             std::declval<Args>()...));                       \
        };                                                                    \
        using method_type = resolved<void(__VA_ARGS__)>::type;                \
        virtual const void* install() {                                       \
            YOMM2_DEBUG(static const char name[] =  #ReturnType " " #Fun  "(" #__VA_ARGS__ ")";) \
            return &::yorel::openmethods::details::Register_spec<decltype(*this), method_type YOMM2_COMMA_DEBUG(name)>::instance; } \
        static ReturnType body(__VA_ARGS__) {

#define end_method } }


#endif
