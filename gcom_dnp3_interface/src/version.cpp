#include <stdio.h>
#include <cstdlib>

#include <version.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// // GIT_BUILD
// extern char _binary_GIT_BUILD_start;
// extern char _binary_GIT_BUILD_end;

// // GIT_COMMIT
// extern char _binary_GIT_COMMIT_start;
// extern char _binary_GIT_COMMIT_end;

// // GIT_TAG
// extern char _binary_GIT_TAG_start;
// extern char _binary_GIT_TAG_end;

Version::Version ()
{
    build = NULL;
    commit = NULL;
    tag = NULL;
}


Version::~Version ()
{
//     if (build != NULL) delete[] build;
//     if (commit != NULL) delete[] commit;
//     if (tag != NULL) delete[] tag;
}

void Version::init(void) {
    build = (char*)TOSTRING(GITVERSION); 
    commit = (char*)TOSTRING(GITCOMMIT);
    tag = (char*)TOSTRING(GITTAG);
}

char* Version::extract(char* info, char* start, char* end)
{
    int iter = 0;
    char* p_iter = start;
    while ( p_iter != end ) {
        p_iter++;
        iter++;
    }
    info = new char[iter];
    snprintf(info, iter, start);
    return info;
}

char* Version::get_build(void)
{
    return build;
}

char* Version::get_commit(void)
{
    return commit;
}

char* Version::get_tag(void)
{
    return tag;
}