# create an image from base centos with systemd enabled
FROM flexgen/centos7:devel

# Install relevant RPMs from artifactory
RUN yum install -y fims-3.0.0 dnp3_interface-3.0.0

WORKDIR /home