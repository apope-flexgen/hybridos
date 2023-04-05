# Here lay the configs for go_logging. 

You can create your own for each executable or use just one. Only three variables are currently configurable. 

See loggerConfig.json for a template. 

Then place them into mcp_xxx.json and change the hybridos_run script to use that mcp_xxx.json

I've provided some folders for frequently used go modules in this directory. They will be used when running ./hybridos_run.sh --verbose. Feel free to modify them for your use cases. 