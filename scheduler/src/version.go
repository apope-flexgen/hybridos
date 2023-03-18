/**
 *
 * version.go
 *
 * Variables and functions for keeping track of the build version.
 *
 */
package main

import "embed"

// Contains release version info
type version struct {
	tag    string
	commit string
	build  string
}

var schedulerVersion version

// The following is a special go directive that embeds the versioning info in strings
//go:embed versioning/GIT_TAG
var tag string

//go:embed versioning/GIT_COMMIT
var commit string

//go:embed versioning/GIT_BUILD
var build string

// Silence unused embed error
//go:embed versioning/GIT_TAG
var _ embed.FS

// Build the version struct from the version info generated by build
func configureVersion() {
	// Assign the embedded versioning info, removing newline characters
	schedulerVersion.tag = tag[:len(tag)-1]
	schedulerVersion.commit = commit[:len(commit)-1]
	schedulerVersion.build = build[:len(build)-1]
}
