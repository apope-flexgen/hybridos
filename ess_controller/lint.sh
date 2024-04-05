# Run this script to run linting checks on all changed files.
# When errors are found during this run, detailed output is suppressed, but cour

tidy() {
    # Check for clang-tidy
    if hash clang-tidy 2>/dev/null; then true;
    else
        echo "clang-tidy command not found. Attempting to install and enable."
        sudo yum install centos-release-scl
        sudo yum install  llvm-toolset-7
        sudo yum install llvm-toolset-7-clang-analyzer llvm-toolset-7-clang-tools-extra
        scl enable llvm-toolset-7 'clang -v'
        scl enable llvm-toolset-7 'lldb -v'
        scl enable llvm-toolset-7 bash
        if hash clang-tidy 2>/dev/null; then
            echo "Install successful"
        else
            echo "Install failed. exiting"
            exit 1
        fi
    fi

    if [ "$1" = "all" ]; then
        printf '\n=== Running clang-tidy on ALL .cpp and .h files ===\n'
        cpp_files=$(find . -type f | grep "\.\(c\|cpp\)\b" | grep -v './doc/' | grep -v './i2c-tools/')
        h_files=$(find . -type f | grep "\.\(h\|hpp\)\b" | grep -v './doc/' | grep -v './i2c-tools/')
    else
        printf '\n=== Running clang-tidy on all changed files ===\n'
        # Regexes designed to only find .ccp and .h files which exist inside of the hybridos/ess_controller directory and which contain changes compared to dev.
        cpp_files=$(git diff --name-status dev | grep -oP '(?<=[AM]\tess_controller\/).*\.(c|cpp)\b' | grep -v './doc/')
        h_files=$(git diff --name-status dev | grep -oP '(?<=[AM]\tess_controller\/).*\.(h|hpp)\b' | grep -v './doc/')
    fi

    failures=""
    total_count=0

    for f in $h_files;
    do
        cmd="clang-tidy $f -- -xc++ -Ifuncs -Iinclude -ISL_files -Isrc"
        echo -e "=== RUN\t${cmd} ==="
        result=$(eval $cmd)
        if [ ${#result} -gt 0 ];
        then
            echo -e "\e[31mFAIL\t$f\e[0m\n"
            failures="$failures $f\n"
        else
            echo -e "\e[32mPASS\t$f\e[0m"
        fi
        ((total_count++))
    done

    for f in $cpp_files;
    do
        cmd="clang-tidy $f -- -Ifuncs -Iinclude -ISL_files -Isrc"
        echo -e "=== RUN\t${cmd} ==="
        result=$(eval $cmd)
        if [ ${#result} -gt 0 ];
        then
            echo -e "\e[31mFAIL\t$f\e[0m\n"
            failures="$failures $f\n"
        else
            echo -e "\e[32mPASS\t$f\e[0m"
        fi
        ((total_count++))
    done

    failed_count=$(( $(echo -e ${failures} | wc -l)-1 ))
    printf '=== Failures: %s/%s ===\n' $failed_count $total_count
    echo -e "\e[31m$failures\t\e[0m"
}

# Run clang-format as well
format () {
    echo "Running clang-format"
    find . -regex '.*\.\(cpp\|hpp\|c\|h\)' -exec clang-format -i {} \;
}

tidy $1
format

exit $failed_count
