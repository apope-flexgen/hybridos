## Jenkinsfile
Copying the Jenkinsfile to the root of another repository that uses package_utility will add that repository to the Jenkins build pipeline. The default values should be reviewed and updated according to your needs. A list of valid inputs for each parameter will be listed below.

buildTrigger: string
-prOnly (Will only trigger a Jenkins build when a PR is opened/updated.)
-allCommits (Will build on every single push to the remote repository)
Example: `buildTrigger = 'prOnly'`

imageNames: string
-centos7
-flexos
Example: `imageNames = ['centos7']`

imageTag: string
-core (only third party RPMs included)
-base (core + latest fims release and fims devel)
-gcc9.2 (base + gcc9.2)
Example: `imageTag = 'base'`

unitTestStage: bool
Example: `unitTestStage = true`

regTestStage: bool
Example: `regTestStage = true`

uploadRepo: string
-flex (fims)
-site-controller-repo (site_controller)
-ess-controller-repo (ess_controller)
-power-cloud-repo (cloud_sync)
-infrastructure (all others)
Example: `uploadRepo = 'infrastructure'`

infRebuild: bool
Example: `infRebuild = false`

## build.sh
Run a build on any flexgen power repository by running ./package_utility/build.sh from the repository's root directory.

In order for build.sh to work correctly, the repository it is running in will need a build_utils.sh script. This should provide a name variable with the name of the repository, a clean function, a build function which puts all build artifacts in the /path/to/repository/build/release directory, an install function, and an uninstall function.

RPM generation will not work correctly unless a name variable is provided and the build function in build_utils.sh puts artifacts to be installed in the /path/to/repository/build/release directory.

## package.sh
package.sh tars up the /path/to/repository/build/release directory and exports it to /path/to/repository/rpmbuild/SOURCES so that an rpm in the appropriate repository can unzip its sources or artifacts correctly. Then package calls rpmbuild repository.spec in order to build a rpm for the repository. The output RPM is placed in /path/to/repository/rpmbuild/RPMS/.

## build_utils.sh
`build_utils.sh` is the file that each repository needs to provide for package_utility to understand _how_ to build and package the repository.

* globals
  * build mode (release, debug, test) can be accessed as "$mode_arg"
  * build output directory (i.e. ./build/release) can be accessed as "$build_output"
* building (./package_utility/build.sh)
  * 'type' (cpp, go, node, etc.) can be optionally specified
  * all artifacts must be output to ./build/[mode]
  * populate 'submodules' to issue builds from within folders in the repository
  * submodule build_utils.sh files need to use the global $build_output for artifacts
* testing (./package_utility/test.sh)
  * elements in 'tests' array are passed to test() as $1 in a looped fashion
* packaging (./package_utility/package.sh)
  * 'name' is required and will be used as the RPM package name
  * package.sh will only generate RPMs from ./build/release
  * create [name].spec file to specifiy software package details. package.sh will pull only one spec file from the module's top-level directory
  * [name].service OR [name]@.service file to specify execution in deployed environment. multiple service files can be added to $build_output directory as needed
* docker (./package_utility/docker.sh)
  * 'image' is required and is used as the Docker repository name, which must already exist (i.e. flexgen/centos7)
  * create a Dockerfile at the top-level directory to generate a Docker image
  * git short tag version is passed to both docker_build() and docker_push() as $1 to use for image tag (i.e. v1.0.0 -> flexgen/centos7:1.0.0)
* error handling
  * add '|| error_trap "error_message"' to critical sections
  * add '|| warning_trap "warning_message"' to issue warnings