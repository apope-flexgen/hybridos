# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum makecache
RUN yum clean all

RUN yum install -y influxdb-1.8.10
RUN yum install -y mongodb-org-6.0.4 mongodb-org-database-6.0.4 mongodb-org-database-tools-extras-6.0.4 mongodb-org-mongos-6.0.4 mongodb-org-server-6.0.4 mongodb-org-tools-6.0.4
RUN yum install -y $productName-$verNum
# do not edit this <<<

# copy over scripts
COPY ./bootstrap/ /home/scripts/
WORKDIR /home/scripts/
RUN chmod +x *.sh
WORKDIR /home/

# expose application ports
# web_server
EXPOSE 443
# influxdb
EXPOSE 8086
# mongodb
EXPOSE 27017

# point to entrypoint script
ENTRYPOINT ["/home/scripts/run.sh"]