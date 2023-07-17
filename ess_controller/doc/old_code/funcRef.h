#ifndef FUNCREF_HPP
#define FUNCREF_HPP

// Used throughout all modules, both internal and non-internal
struct funcRef
{
    const char* func_name;
    void* func_ptr;
};

#endif