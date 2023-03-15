#!/bin/bash
name=dnp3_interface
gcc=9
type=cpp
modes=(release debug test)
tests=()

function clean() {
   echo "clean function called"
   make clean
   #rm -rf build
}

function prebuild() {
   echo "prebuild function called (empty)"
}

function build() {   # build.sh passes build mode as $1
   echo "build function called"
   make BUILD_MODE=$1
   cp dnp3*.service build/$1
   cp build/$1_obj/*.so  build/$1
 
  #!/bin/bash
}

function postbuild() {
   echo "postbuild function called (empty)"
}

function install() # build.sh passes build mode as $1
{
    sudo make BUILD_MODE=$1 install
}

function uninstall()
{
    sudo make uninstall

}
