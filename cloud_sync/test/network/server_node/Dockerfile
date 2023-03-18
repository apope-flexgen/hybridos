# base image (should probably set this to imageName=centos7 and imageTag=devel in most use cases)
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

RUN yum clean all

# install openssh-server, needed for cloud_sync to transfer files externally
RUN yum install -y openssh-server

# tools for modifying the network
RUN yum install -y iproute

RUN mkdir -p /home/archives
RUN mkdir -p /home/local_consumer
RUN mkdir -p /home/config/
RUN mkdir -p /home/.cloud_sync/
RUN mkdir -p /var/log/flexgen/cloud_sync/

# setup ssh server
RUN mkdir -p /root/.ssh/
RUN ssh-keygen -A
EXPOSE 22

ENTRYPOINT ["/home/scripts/run.sh"]