# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum clean all
# do not edit this <<<

RUN mkdir -p /home/config/
RUN mkdir -p /home/.cloud_sync/
RUN mkdir -p /root/.ssh/
RUN mkdir -p /var/log/flexgen/cloud_sync/

# install openssh-clients, needed for cloud_sync to transfer files externally
RUN yum install -y openssh-clients

# install cloud_sync at specific version
RUN yum install -y cloud_sync-$verNum

COPY ./scripts/run.sh /home/scripts/
RUN chmod +x /home/scripts/run.sh

ENTRYPOINT ["/home/scripts/run.sh"]