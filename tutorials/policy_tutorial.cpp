#include <boost/mpl/print.hpp>

#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/runtime.hpp>
#include <yorel/yomm2/symbols.hpp>

using namespace yorel::yomm2;
using namespace policy;

class Animal {
  public:
    virtual ~Animal() {
    }
};

class Cat : public Animal {};
class Dog : public Animal {};

struct kick_key;

namespace minimal {

// struct minimal_policy : generic_policy<minimal_policy>, rtti {
//     template<typename T>
//     static type_id static_type() {
//         return reinterpret_cast<type_id>(&static_vptr<T>);
//     }
// };

BOOST_AUTO_TEST_CASE(policy_tutorial_minimal_policy) {

    struct minimal_policy : generic_policy<minimal_policy>,
                            minimal_rtti<minimal_policy> {};
    static use_classes<minimal_policy, Animal, Cat, Dog> YOMM2_GENSYM;

    using kick = method<
        minimal_policy, kick_key,
        std::string(virtual_ptr<minimal_policy, Animal>)>;

    update<minimal_policy>();
    BOOST_TEST(minimal_policy::dispatch_data.size() == 3);
    BOOST_TEST(
        minimal_policy::dispatch_data[0] ==
        reinterpret_cast<std::uintptr_t>(kick::not_implemented_handler));

    struct kick_dog {
        static std::string fn(virtual_ptr<minimal_policy, Dog> dog) {
            return "bark";
        }
    };

    struct kick_cat {
        static std::string fn(virtual_ptr<minimal_policy, Cat> dog) {
            return "hiss";
        }
    };

    static kick::add_definition<kick_cat> YOMM2_GENSYM;
    static kick::add_definition<kick_dog> YOMM2_GENSYM;

    update<minimal_policy>();
    // BOOST_TEST(
    //     minimal_policy::dispatch_data[1] ==
    //     reinterpret_cast<std::uintptr_t>(kick_cat::fn));

    Cat&& cat = Cat();
    Dog&& dog = Dog();

    std::vector<virtual_ptr<minimal_policy, Animal>> animals = {
        final_virtual_ptr<minimal_policy>(cat),
        final_virtual_ptr<minimal_policy>(dog),
    };

    BOOST_TEST(kick::fn(animals[0]) == "hiss");
    BOOST_TEST(kick::fn(animals[1]) == "bark");
}
} // namespace minimal
