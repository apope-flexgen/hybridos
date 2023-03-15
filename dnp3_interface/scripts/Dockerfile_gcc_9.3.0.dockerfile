FROM centos:7 AS env

RUN yum update -y
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-9
RUN yum install -y libmodbus net-tools iproute openssh-clients openssh-server wget nano  git jq 



RUN echo "source /opt/rh/devtoolset-9/enable" >> /etc/bashrc
SHELL ["/bin/bash", "--login", "-c"]
RUN gcc --version
#docker run  -it -v //c/Users/17576/flexgen/dockerbuild/gcctest2:/home/build -d gcctest2
#docker build -t gcctest2 .