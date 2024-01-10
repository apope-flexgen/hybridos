# capture working branch
branch=$(git branch | grep \* | cut -d ' ' -f2)
echo -e "branch:\t\t$branch"
rm -f GIT_BRANCH
echo "$branch" >> GIT_BRANCH
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_BRANCH git_branch.o

# capture current commit
commit=$(git log --pretty=format:'%h' -n 1)
echo -e "commit:\t\t$commit"
rm -f GIT_COMMIT
echo "$commit" >> GIT_COMMIT
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_COMMIT git_commit.o

# capture latest tag
if git describe --match v* --abbrev=0 --tags HEAD &> /dev/null ; then
    tag_long=$(git describe --match "v*" --abbrev=0 --tags HEAD)
    if [[ $tag_long == "v"* ]]; then tag=${tag_long:1}; fi # (v1.0.0) -> (1.0.0)
    if [[ $tag_long == *"-rc" ]]; then tag=$(echo $tag | cut -d'-' -f 1); rc=".rc"; fi # (v1.0.0-rc) - > (v.1.0.0.rc)

else
    tag=$commit # no tag info, use abbreviated commit hash
fi
echo -e "tag:\t\t$tag"
rm -f GIT_TAG
echo "$tag" >> GIT_TAG
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_TAG git_tag.o

# capture current build number
# TODO: override BUILD during Jenkins build
if [ ! -n "$BUILD" ]; then
    BUILD=$(git rev-list --count "$commit")
fi
echo -e "build:\t\t$BUILD"
rm -f GIT_BUILD
echo "$BUILD" >> GIT_BUILD
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_BUILD git_build.o
