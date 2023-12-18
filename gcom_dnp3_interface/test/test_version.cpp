// access version info
#include <version.h>



//OBJECTS += version.o git_build.o git_commit.o git_tag.o Configurator.o LDSS.o Input_Sources.o Logger.o

// # capture working branch
// branch=$(git branch | grep \* | cut -d ' ' -f2)
// echo -e "branch:\t\t$branch"
// rm -f GIT_BRANCH
// echo "$branch" >> GIT_BRANCH
// objcopy --input binary \
//             --output-target elf64-x86-64 \
//             --binary-architecture i386 GIT_BRANCH git_branch.o

// # capture current commit
// commit=$(git log --pretty=format:'%h' -n 1)
// echo -e "commit:\t\t$commit"
// rm -f GIT_COMMIT
// echo "$commit" >> GIT_COMMIT
// objcopy --input binary \
//             --output-target elf64-x86-64 \
//             --binary-architecture i386 GIT_COMMIT git_commit.o

// # capture latest tag
// if git describe --match v* --abbrev=0 --tags HEAD &> /dev/null ; then
//     tag_long=$(git describe --match "v*" --abbrev=0 --tags HEAD)
//     if [[ $tag_long == "v"* ]]; then tag=${tag_long:1}; fi # (v1.0.0) -> (1.0.0)
//     if [[ $tag_long == *"-rc" ]]; then tag=$(echo $tag | cut -d'-' -f 1); rc=".rc"; fi # (v1.0.0-rc) - > (v.1.0.0.rc)

// else
//     tag=$commit # no tag info, use abbreviated commit hash
// fi
// echo -e "tag:\t\t$tag"
// rm -f GIT_TAG
// echo "$tag" >> GIT_TAG
// objcopy --input binary \
//             --output-target elf64-x86-64 \
//             --binary-architecture i386 GIT_TAG git_tag.o

// # capture current build number
// # TODO: override BUILD during Jenkins build
// if [ ! -n "$BUILD" ]; then
//     BUILD=$(git rev-list --count "$commit")
// fi
// echo -e "build:\t\t$BUILD"
// rm -f GIT_BUILD
// echo "$BUILD" >> GIT_BUILD
// objcopy --input binary \
//             --output-target elf64-x86-64 \
//             --binary-architecture i386 GIT_BUILD git_build.o


//./package_utility/version.sh

// ##### version
// name:           gcom_interface
// branch:         stephanie/handle_fims_input
// commit:         bd5655f
// tag:            bd5655f.component
// build:          46
// rpmbuild:       gcom_interface-bd5655f.component-46.local

//g++ -g --std=c++17 -o build/release/tv -I include test/test_version.cpp git*.o  -lpthread
// sh-4.2# ./build/release/tv
// 46
// bd5655f
// bd5655f.component
// build: 46
// commit: bd5655f
// tag: bd5655f.component
/*
 * Asset.cpp
 *
 *  Created on: 2021-02-03
 *      Author: Anirudh Mulukutla
 */
#include <stdio.h>
#include <cstdlib>

#include <version.h>

// GIT_BUILD
extern char _binary_GIT_BUILD_start;
extern char _binary_GIT_BUILD_end;

// GIT_COMMIT
extern char _binary_GIT_COMMIT_start;
extern char _binary_GIT_COMMIT_end;

// GIT_TAG
extern char _binary_GIT_TAG_start;
extern char _binary_GIT_TAG_end;

Version::Version ()
{
    build = NULL;
    commit = NULL;
    tag = NULL;
}

Version::~Version ()
{
    if (build != NULL) delete[] build;
    if (commit != NULL) delete[] commit;
    if (tag != NULL) delete[] tag;
}

void Version::init(void) {
    build = extract(build, &_binary_GIT_BUILD_start, &_binary_GIT_BUILD_end);
    commit = extract(commit, &_binary_GIT_COMMIT_start, &_binary_GIT_COMMIT_end);
    tag = extract(tag, &_binary_GIT_TAG_start, &_binary_GIT_TAG_end);
}

char* Version::extract(char* info, char* start, char* end)
{
    int iter = 0;
    char* p_iter = start;
    while ( p_iter != end ) {
        putchar(*p_iter++);
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

int main ()
{


    Version* version = new Version();
    version->init();
    printf("build: %s\n", version->get_build());
    printf("commit: %s\n", version->get_commit());
    printf("tag: %s\n", version->get_tag());

   return 0;
}