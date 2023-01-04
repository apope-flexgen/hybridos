# FlexGen Internal Messaging Service

FIMS is the primary messaging service for HybridOS.

## Getting Started


## pyfims wrapper

### Getting ready
sudo yum install -y python36 python36-pip python36-devel
sudo pip3 install pandas cython

### Compiling the python module .so
python3 setup.py build_ext --inplace

### Use
python3
import pyfims