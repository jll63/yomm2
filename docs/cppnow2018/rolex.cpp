// clang++-5.0 -std=c++17 -I ~/dev/yomm2/include -O3 -DNDEBUG -save-temps -o rolex rolex.cpp ~/dev/yomm2/src/yomm2.cpp && ./rolex
// clang++-5.0 -std=c++17 -I ~/dev/yomm2/include -E rolex.cpp | clang-format | perl -pe 's/::yorel::yomm2::(detail::)?//'

// clang++-6.0 -std=c++17 -I ~/dev/yomm2/include -o rolex rolex.cpp ~/dev/yomm2/src/yomm2.cpp && YOMM2_TRACE=1 ./rolex


#include <yorel/yomm2/cute.hpp>

using yorel::yomm2::virtual_;

struct Role {
    virtual ~Role() {}
};

struct Employee : Role {
    virtual double pay() const;
};

struct Manager : Employee {
    virtual double pay() const;
};

struct Founder : Role {};

struct Expense {
    virtual ~Expense() {}
};

struct Public : Expense {};
struct Bus : Public {};
struct Metro : Public {};
struct Taxi : Expense {};
struct Jet : Expense {};

register_class(Role);
register_class(Employee, Role);
register_class(Manager, Employee);
register_class(Founder, Role);
register_class(Expense);
register_class(Public, Expense);
register_class(Bus, Public);
register_class(Metro, Public);
register_class(Taxi, Expense);
register_class(Jet, Expense);

declare_method(double, pay, (virtual_<const Employee&>));
declare_method(bool, approve, (virtual_<const Role&>, virtual_<const Expense&>, double));

define_method(double, pay, (const Employee&)) {
    return 3000;
}

define_method(double, pay, (const Manager& exec)) {
    return next(exec) + 2000;
}

define_method(bool, approve, (const Role& r, const Expense& e, double amount)) {
    return false;
}

define_method(bool, approve, (const Employee& r, const Public& e, double amount)) {
    return true;
}

define_method(bool, approve, (const Manager& r, const Taxi& e, double amount)) {
    return true;
}

define_method(bool, approve, (const Founder& r, const Expense& e, double amount)) {
    return true;
}

int main() {
    yorel::yomm2::update_methods();
}

double call_pay(const Employee& emp) {
    return pay(emp);
}

double Employee::pay() const {
    return 3000;
}

double Manager::pay() const {
    return Employee::pay() + 2000;
}

double call_pay_vfunc(const Employee& emp) {
    return emp.pay();
}
