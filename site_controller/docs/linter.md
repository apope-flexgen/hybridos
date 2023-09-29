# Linting in site_controller
site_controller files are linted to conform to a standard format to encourage clean coding practices. This is lifted straight from Google's formatting practices.

To lint files, run
```
sh ./lint.sh
```

## What the script does
The script is a wrapper around the `clang-tidy` command, which invokes the clang-tidy binary on any files that are found with active changes. It does this by running a git status and looking for any .cpp and .h files, then checking them for errors and printing if they fail. Here is an example of passing output:
```
[hybridos@localhost site_controller]$ ./lint.sh

=== Running clang-tidy on all changed files ===
=== RUN clang-tidy -fix include/Asset_Cmd_Object.h -- -xc++ -Iinclude -Isrc ===
PASS    include/Asset_Cmd_Object.h
=== RUN clang-tidy -fix src/Asset_Cmd_Object.cpp -- -Iinclude ===
17 warnings generated.
Suppressed 17 warnings (17 with check filters).
PASS    src/Asset_Cmd_Object.cpp
=== Failures: 0/2 ===
```
Sometimes clang-tidy is smart enough to fix any problems it finds, but if not, it will report failures such as in the following:
```
[hybridos@localhost site_controller]$ ./lint.sh

=== Running clang-tidy on all changed files ===
=== RUN clang-tidy -fix include/Asset_Cmd_Object.h -- -xc++ -Iinclude -Isrc ===
PASS    include/Asset_Cmd_Object.h
=== RUN clang-tidy -fix src/Asset_Cmd_Object.cpp -- -Iinclude ===
17 warnings and 1 error generated.
Error while processing /home/hybridos/git/hybridos/site_controller/src/Asset_Cmd_Object.cpp.
Suppressed 17 warnings (17 with check filters).
Found compiler errors, but -fix-errors was not specified.
Fixes have NOT been applied.

FAIL    src/Asset_Cmd_Object.cpp

=== Failures: 1/2 ===
 src/Asset_Cmd_Object.cpp

```


## Which rules are we using?
The rules can be configured in the first line of `.clang-tidy`, which follows this format.
```
Checks: 'clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-optin.performance.Padding,-clang-diagnostic-inconsistent-missing-override,-clang-analyzer-core.UndefinedBinaryOperatorResult'
```

In general we are including all of the diagnostic and analyzer checks, then ignoring a couple other ones.

## Making it happen automatically (format on save)
1. First ensure that you have run `./lint.sh` inside of this repo once, to install the tooling.

2. Next add the following to your `.bashrc` or `.bash_profile`:
```
source /opt/rh/llvm-toolset-7/enable
```
3. Then install the vscode extension [RunOnSave](https://marketplace.visualstudio.com/items?itemName=emeraldwalk.RunOnSave) in vscode.

4. Next open preferences (ctrl+shift+P) and select `Preferences: Open User Settings (JSON)`

5. Last, append the following items to the JSON:
```
"emeraldwalk.runonsave": {
    "commands": [
        {
            "match": "\\.(h\b|hpp\b|c\b|cpp\b)",
            "cmd": "clang-format ${file}"
        }
    ]
},
"[cpp]": {
    "editor.wordBasedSuggestions": false,
    "editor.suggest.insertMode": "replace",
    "editor.semanticHighlighting.enabled": true,
    "editor.defaultFormatter": "xaver.clang-format"
    "editor.formatOnSave": true
},
```

Note: Ensure you don't have another entry for `[cpp]`. If you do, you could upsert the properties into what you already have.

After this, anytime you save a file with extension .c, .h, .cpp, or .hpp it should automatically format.
