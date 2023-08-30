/*
 * Asset.cpp
 *
 *  Created on: 2021-02-03
 *      Author: Anirudh Mulukutla
 */
#include <version.h>

#include <cstdio>
#include <cstdlib>

// GIT_BUILD
extern char _binary_GIT_BUILD_start;
extern char _binary_GIT_BUILD_end;

// GIT_COMMIT
extern char _binary_GIT_COMMIT_start;
extern char _binary_GIT_COMMIT_end;

// GIT_TAG
extern char _binary_GIT_TAG_start;
extern char _binary_GIT_TAG_end;

Version::Version() {
    build = NULL;
    commit = NULL;
    tag = NULL;
}

Version::~Version() {
    { delete[] build; }
    { delete[] commit; }
    { delete[] tag; }
}

void Version::init(void) {
    build = extract(build, &_binary_GIT_BUILD_start, &_binary_GIT_BUILD_end);
    commit = extract(commit, &_binary_GIT_COMMIT_start, &_binary_GIT_COMMIT_end);
    tag = extract(tag, &_binary_GIT_TAG_start, &_binary_GIT_TAG_end);
}

char* Version::extract(char* info, char* start, const char* end) {
    int iter = 0;
    char* p_iter = start;
    while (p_iter != end) {
        putchar(*p_iter++);
        iter++;
    }
    info = new char[iter];
    snprintf(info, iter, "%s", start);
    return info;
}

char* Version::get_build(void) {
    return build;
}

char* Version::get_commit(void) {
    return commit;
}

char* Version::get_tag(void) {
    return tag;
}
