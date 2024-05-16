FROM flexgen/centos7:devel

RUN ["bin/bash", "-c", "echo 'export PATH=/opt/rh/devtoolset-9/root/bin:$PATH' >> ~/.bashrc"]
RUN yum install -y fims-10.3.15
RUN ["mkdir", "-p", "/usr/local/etc/certs/gcom_dnp3_interface"]
RUN ["bin/bash", "-c", "echo 'if [[ ! $(ps ax | grep -v grep | grep fims_server) ]] ; then \n fims_server > /dev/null 2>&1 & \n fi \n' >> ~/.bashrc"]

WORKDIR /home/docker/hybridos

ENTRYPOINT ["/bin/bash"]