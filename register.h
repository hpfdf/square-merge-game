// File: register.h
//
// Brief: Macro-free registration utilities. The interface that enables a base
//        class to create child classes by name, with custom but uniform
//        constructor arguments. C++11 is required.
//
// Author: Pufan He <hpfdf@126.com>
//
// Version: 2014/08/21
//
// Usage: Write your own Base class extending
//            RegisterBase<Base, constructor_arguments_types...>.
//        Write your own Child classes extending
//            Register<Child, Base, registered_name>.
//        Then you can create new Child by a string of registered name.
//
//        class Base: RegisterBase<Base, Argument...>
//
//          static Base* Create(const string& name, Argument... args);
//          >> Create a new registered child class of Base. i.e.,
//                 new Child(args...)
//             will be returned, where Child is registered by the name.
//          >> If no class is created in case of error, return nullptr.
//          >> Caller takes ownership of returned pointer.
//          >> args can be empty or any function argument configures, e.g.
//                 class Base: RegisterBase<Base, int, string> {
//                  public:
//                   Base(int id, string msg) {...}
//                   ...
//                 }
//          >> It also has the following creator variations. The memory of
//             created objects Will be automatically cleared if using these:
//                 static Base::uptr CreateUnique(name, ...);
//                 static Base::ptr CreateShared(name, ...);
//             where
//                 Base::uptr = std::unique_ptr<Base>;
//                 Base::ptr = std::shared_ptr<Base>;
//
//          static bool HasChild(const string& name);
//          >> Return whether a name is registered by any child class of Base.
//
//          static bool SetChild<Child>(const string& name);
//          >> Manually register a child class by name. Will fail if try to
//             register a duplicated name, or Child is not Registered.
//          >> Return successful or not.
//
//          static bool RemoveChild(const string& name);
//          >> Remove a registered name, so Base cannot create the associated
//             child with name after this is called.
//          >> Return true if a child is removed.
//
//          static vector<string> GetChildren();
//          >> Return a list of names of all current registered children, in
//             alphabetic order.
//
//          virtual const char* name();
//          >> Get the runtime name of this object which is or extends Base.
//
//          virtual const char* info();
//          >> Get the introduction of this registered object.
//
//        class Child: Register<Child, Base, Name>
//
//          Base's interface;
//          >> Register<Child, Base, Name> itself extends Base, and thus
//             Base::Create(Name) is able to correctly return the Child as a
//             Base*, because Child extends Register<Child, Base, Name>.
//             Specially, when Name is empty, i.e.,
//                 class Child: Register<Child, Base>
//             The Base class does not register the Child class for create.
//             However, user may use SetName() later to enable the register.
//
//             Child must have constructor functions with all configures of
//             arguments which aer declared in all RegisterBases extended by
//             Base.
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
//         class Fruit: RegisterBase<Fruit> {...};
//         class Apple: Register<Apple, Fruit, "Apple"> {...};
//         class Banana: Register<Banana, Fruit, "Banana"> {...};
//
//         template <int i>
//         class FruitInBox: Register<FruitInBox<i>, Fruit> {...};
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

#ifndef REGISTER_H_
#define REGISTER_H_

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

template <class Base, typename... Argument>
struct RegisterCreatorBase {
  virtual Base* Create(Argument...) {
    return nullptr;
  }
};

template <class Child, class Base, typename... Argument>
struct RegisterCreator: RegisterCreatorBase<Base, Argument...> {
  Base* Create(Argument... args) override {
    return new Child(args...);
  }
};

template <class Child, class Base, typename... Argument>
class RegisterCreatorContainer{
 public:
  static RegisterCreatorBase<Base, Argument...>* GetCreator() {
    return &creator_;
  }
 private:
  static RegisterCreator<Child, Base, Argument...> creator_;
};

template <class Base, typename... Argument>
class RegisterBase {
 public:
  typedef std::unique_ptr<Base> uptr;
  typedef std::shared_ptr<Base> ptr;
  typedef std::unordered_map<std::string, RegisterCreatorBase<Base, Argument...>*>
      RegisterTable;

  static Base* Create(const std::string& name, Argument... args) {
    if (!creators_.count(name))
      return nullptr;
    else
      return creators_[name]->Create(args...);
  }

  static uptr&& CreateUnique(const std::string& name, Argument... args) {
    return uptr(Create(name, args...));
  }

  static ptr&& CreateShared(const std::string& name, Argument... args) {
    return ptr(Create(name, args...));
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

  static std::vector<std::string> GetChildren() {
    std::vector<std::string> result;
    for (const auto& it: creators_)
      result.push_back(it->first);
    return result;
  }

  virtual const char* name() {
    return "";
  }

  virtual const char* info() {
    return "Register base class.";
  }

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
        Base::SetChild<Child>(Name);
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

  virtual const char* info() override {
    return ("Registered sub-class \"" + name_ + "\".").c_str();
  };

  const char* name() override {
    return name_.c_str();
  }

 private:
  void activate() {
    if (Name != nullptr)
      &activator_;
  }

  static RegisterActivator<Child, Base, Name> activator_;
  static std::string name_;
};

template <class Base, typename... Argument>
typename RegisterBase<Base, Argument...>::RegisterTable
RegisterBase<Base, Argument...>::creators_;

template <class Child, class Base, typename... Argument>
RegisterCreator<Child, Base, Argument...>
RegisterCreatorContainer<Child, Base, Argument...>::creator_;

template <class Child, class Base, const char* Name>
RegisterActivator<Child, Base, Name>
Register<Child, Base, Name>::activator_;

#endif  // REGISTER_H_
