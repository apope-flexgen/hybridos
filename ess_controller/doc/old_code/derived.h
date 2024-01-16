//Here is a complete working example:

//Interface declaration: Interface.h

class Base {
public:
    virtual ~Base() {}
    virtual void foo() const = 0;
};

//using Base_creator_t = Base *(*)();
//Shared library content:

#include "Interface.h"

class Derived: public Base {
public:
    void foo() const override {}
};

extern "C" {
    Base * create() {
        return new Derived;
    }
}

Dynamic shared library handler:
Derived_factory.hpp:

#include "Interface.hpp"
#include <dlfcn.h>

class Derived_factory {
public:
    Derived_factory() {
        handler = dlopen("libderived.so", RTLD_NOW);
        if (! handler) {
            throw std::runtime_error(dlerror());
        }
        Reset_dlerror();
        creator = reinterpret_cast<Base_creator_t>(dlsym(handler, "create"));
        Check_dlerror();
    }

    std::unique_ptr<Base> create() const {
        return std::unique_ptr<Base>(creator());
    }

    ~Derived_factory() {
        if (handler) {
            dlclose(handler);
        }
    }

private:
    void * handler = nullptr;
    Base_creator_t creator = nullptr;

    static void Reset_dlerror() {
        dlerror();
    }

    static void Check_dlerror() {
        const char * dlsym_error = dlerror();
        if (dlsym_error) {
            throw std::runtime_error(dlsym_error);
        }
    }
};
Client code:

#include "Derived_factory.hpp"

{
    Derived_factory factory;
    std::unique_ptr<Base> base = factory.create();
    base->foo();
}
