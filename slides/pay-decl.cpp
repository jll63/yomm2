// (setq c-basic-offset 2)

struct _yomm2_method_pay;
namespace { namespace YoMm2_nS_10 {
using _yOMM2_method =
  method<void, _yomm2_method_pay,
         double(virtual_<const Employee &>),
         default_policy>;
_yOMM2_method::init_method init = "pay"
  "(virtual_<const Employee&>)";
}
}
YoMm2_nS_10::_yOMM2_method
pay(discriminator,
    virtual_arg_t<virtual_<const Employee &>> a0);
inline double
pay(virtual_arg_t<virtual_<const Employee &>> a0) {
  auto pf = reinterpret_cast<double (*)(
    virtual_arg_t<virtual_<const Employee &>> a0)>(
      YoMm2_nS_10::_yOMM2_method::resolve(a0));
  return pf(std::forward<
            virtual_arg_t<virtual_<const Employee &>>>(
              a0));
};
