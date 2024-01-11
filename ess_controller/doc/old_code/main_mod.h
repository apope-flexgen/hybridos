

#ifndef MAIN_MOD_HPP
#define MAIN_MOD_HPP


#include <iostream>
#include <dlfcn.h>
#ifdef __GNUC__
__extension__
#endif
// isprovider_t *hasType = reinterpret_cast<isprovider_t*>(myLib->resolve("type"));
#include <stdio.h>

//g++ -o main_mod main_mod.cpp -ldl

using std::cout;
using std::cerr;
const char* base_dir = "./libs";

typedef struct module_t {
public:
    module_t( const char *mname)
    {
        name = mname;
        get_module(mname);
    }
    // unload the  library
    ~module_t()
    {
        dlclose(item);
    }
    void get_module (const char* _name)
    {
        char mname [1024];
        snprintf(mname, sizeof (mname),"%s/lib%s.so", base_dir, _name);
        item = dlopen(mname, RTLD_LAZY);
        if (!item)
        {
            cerr << "Cannot load library: " << dlerror() << '\n';
            // todo fix errors
        }
        else
        {
            // reset errors
            dlerror();
            create = (create_t*)dlsym(item, "create");
            const char* dlsym_error = dlerror();
            if (dlsym_error)
            {
                cerr << "Cannot load symbol create: " << dlsym_error << '\n';
                create = nullptr;
                return;
            }

            destroy = (destroy_t*) dlsym(item, "destroy");
            dlsym_error = dlerror();
            if (dlsym_error)
            {
                cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
                destroy = nullptr;
                return;
            }
            createm = (createm_t*) dlsym(item, "createm");
            dlsym_error = dlerror();
            if (dlsym_error)
            {
                cerr << "Cannot load symbol createm: " << dlsym_error << '\n';
                createm = nullptr;
                return;
            }

            destroym = (destroym_t*) dlsym(item, "destroym");
            dlsym_error = dlerror();
            if (dlsym_error)
            {
                cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
                destroym = nullptr;
                return;
            }

        }
    }

    std::string  name;
    void *item;
    create_t *create;
    destroy_t *destroy;
    createm_t *createm;
    destroym_t *destroym;

} module;
#endif
