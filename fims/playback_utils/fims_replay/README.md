## Usage

`fims_replay` reads in a text file obtained from `fims_listen` and will send out all of the same messages with approximately the same relative timing. (Notably, the username and process name will be different, but the method, uri, and message body will be the same.) Works up to approximately 0.5 millisecond resolution.

```
fims_replay path/to/fims_listen_log.txt
```

## Obtaining a fims_listen text file

With fims installed, use the following command:
```
fims_listen >> path/to/output_file.txt
```

## Build

Should be able to remake the binary with a simple `make` command.