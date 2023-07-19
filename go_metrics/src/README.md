# The `/src` Directory

`main.go` contains the main run script for go_metrics. It depends strongly on the `go_metrics` package found in `pkg/go_metrics`. You will first need to copy the `config` and `go_metrics` packages to GOROOT (typically `/usr/local/go` or `usr/lib/golang`):
```
\cp -r ../pkg/go_metrics [GOPATH]/src
cp -a ../pkg/go_metrics  [GOPATH]/src
```

To create a local build of the package, run the following command in the terminal:
```
go build -o go_metrics
```

Then, to run the program:
```
./go_metrics [path/to/config]
```

See the main Go Metrics documentation for more details about features and functionality.