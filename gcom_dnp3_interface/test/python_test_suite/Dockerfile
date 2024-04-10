FROM flexgen/centos7:devel-env

RUN yum install -y fims
RUN ["mkdir", "-p", "/usr/local/etc/certs/gcom_dnp3_interface"]
RUN ["bin/bash", "-c", "echo 'if [[ ! $(ps ax | grep -v grep | grep fims_server) ]] ; then \n fims_server > /dev/null 2>&1 & \n fi \n' >> ~/.bashrc"]

WORKDIR /home/docker/hybridos/gcom_dnp3_interface/test/python_test_suite

ENTRYPOINT ["/bin/bash"]