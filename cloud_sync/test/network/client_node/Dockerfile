# base image (should probably set this to imageName=centos7 and imageTag=devel in most use cases)
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

RUN yum clean all

# install openssh-clients, needed for cloud_sync to transfer files externally
RUN yum install -y openssh-clients

# tools for modifying the network
RUN yum install -y iproute

RUN mkdir -p /home/archives
RUN mkdir -p /home/local_consumer
RUN mkdir -p /home/config/
RUN mkdir -p /home/.cloud_sync/
RUN mkdir -p /var/log/flexgen/cloud_sync/

# setup ssh client
RUN mkdir -p /root/.ssh/

ENTRYPOINT ["/home/scripts/run.sh"]