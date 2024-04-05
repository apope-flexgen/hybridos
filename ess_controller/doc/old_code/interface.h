// Interface declaration: Interface.h

class Base
{
public:
    virtual ~Base() {}
    virtual void foo() const = 0;
};

using Base_creator_t = Base* (*)();
