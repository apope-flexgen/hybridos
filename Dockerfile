# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum clean all
# do not edit this <<<

RUN yum install -y $productName-$verNum

# copy over scripts
COPY ./scripts/$dockerName /home/scripts/
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