// File: register.h
//
// Brief: Macro-free registration utilities. The interface that enables a base
//        class to create child classes by name, with no generator arguments.
//        C++11 is required.
//
// Author: Pufan He <hpfdf@126.com>
//
// Version: 2014/08/19
//
// Usage: Write your own Base or Child classes that extend RegisterBase or
//        Register to enable the create-by-name feature.
//
//        class Base : RegisterBase<Base>
//
//          static Base* Create(const string& name);
//          >> Create a new registered child class of Base. If name is empty,
//             new Base is returned. Caller takes ownership of returned
//             pointer. If no class is created, return nullptr.
//
//          static bool HasChild(const string& name);
//          >> Return whether a name is registered by any child class of Base.
//
//          static bool SetChild<Child>(const string& name);
//          >> Manually register a child class by name. Will fail if try to
//             register a duplicated name. Return successful or not.
//
//          static bool RemoveChild(const string& name);
//          >> Remove a registered name, so Base cannot create the associated
//             child with name. Return true if a child is removed.
//
//        class Child : Register<Child, Base, Name>
//
//          Base's interface;
//          >> Register<Child, Base, Name> itself extends Base, and thus
//             Base::Create(Name) is able to correctly return the Child as a
//             Base*, because Child extends Register<Child, Base, Name>.
//             Specially, when Name is empty, i.e.,
//                 class Child : Register<Child, Base>
//             The Base class does not register the Child class for create.
//             However, user may use SetName() later to enable the register.
//
//          static const char* GetName();
//          >> Get the registered name of this child class.
//
//          static bool SetName(const string& name);
//          >> Reset the registered name of this child class. The original
//             registration (if exists) will be removed and no longer available.
//             The new name will be registered for Create. Return whether
//             successful. If return false, the name will not change.
//
// Sample: (definitions:)
//
//         #include "register.h"
//
//         class Fruit : RegisterBase<Fruit> {...};
//         class Apple : Register<Apple, Fruit, "Apple"> {...};
//         class Banana : Register<Banana, Fruit, "Banana"> {...};
//
//         template <int i>
//         class FruitInBox : Register<FruitInBox<i>, Fruit> {...};
//
//         (usages:)
//
//         Fruit* apple = Fruit::Create("Apple");
//         Fruit::HasChild("Banana") == true
//         Fruit::Create("BadApple") == nullptr
//
//         for (int i = 0; i < 10; ++i)
//           FruitInBox<i>::SetName("FruitInBox" + std::to_string(i));
//         Fruit::Create("FruitInBox3");
//

#include <unordered_map>
#include <string>

namespace {

template <class Base>
struct RegisterCreatorBase {
  virtual Base* Create() {
    return nullptr;
  }
};

template <class Child, class Base>
struct RegisterCreator: RegisterCreatorBase<Base> {
  Base* Create() override {
    return new Child;
  }
};

template <class Child, class Base>
class RegisterCreatorContainer{
 public:
  static RegisterCreatorBase<Base>* GetCreator() {
    return &creator_;
  }
 private:
  static RegisterCreator<Child, Base> creator_;
};

template <class Base>
class RegisterBase {
 public:
  static Base* Create(const std::string& name = "") {
    if (name.empty())
      return new Base;
    if (!creators_.count(name))
      return nullptr;
    else
      return creators_[name]->Create();
  }

  static bool HasChild(const std::string& name) {
    return creators_.count(name);
  }

  static bool RemoveChild(const std::string& name) {
    if (HasChild(name)) {
      creators_.erase(creators_.find(name));
      return true;
    }
    return false;
  }

  template <class Child>
  static bool SetChild(const std::string& name) {
    if (!name.empty()) {
      if (HasChild(name))
        return false;
      creators_[name] = RegisterCreatorContainer<Child, Base>::GetCreator();
      return true;
    }
    return false;
  }

  typedef std::unordered_map<std::string, RegisterCreatorBase<Base>*>
      RegisterTable;
 private:
  static RegisterTable creators_;
};

template <class Child, class Base, const char* Name>
class RegisterActivator {
 public:
  RegisterActivator() {
    static bool registered = false;
    if (!registered) {
      registered = true;
      if (Name != nullptr)
        RegisterBase<Base>::template SetChild<Child>(Name);
    }
  }
};

template <class Child, class Base, const char* Name = nullptr>
class Register: public Base {
 public:
  Register() {
    if (Name != nullptr)
      name_ = Name;
    else
      name_ = "";
  }

  static const char* GetName() {
    return name_.c_str();
  }

  static bool SetName(const std::string& name) {
    Base::RemoveChild(name_);
    if (Base::SetChild<Child>(name)) {
      name_ = name;
      return true;
    }
    return false;
  }

 private:
  void activate() {
    if (Name != nullptr)
      &activator_;
  }

  static RegisterActivator<Child, Base, Name> activator_;
  static std::string name_;
};

template <class Base>
typename RegisterBase<Base>::RegisterTable RegisterBase<Base>::creators_;

template <class Child, class Base>
RegisterCreator<Child, Base> RegisterCreatorContainer<Child, Base>::creator_;

template <class Child, class Base, const char* Name>
RegisterActivator<Child, Base, Name> Register<Child, Base, Name>::activator_;

}  // namespace
