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
RUN mkdir -p /home/data/
RUN mkdir -p /home/.error/
RUN mkdir -p /var/log/flexgen/dts/
RUN mkdir -p /var/lib/influxdb/
RUN mkdir -p /var/lib/mongo/

COPY ./scripts/run.sh /home/scripts/
RUN chmod +x /home/scripts/run.sh

ENTRYPOINT ["/home/scripts/run.sh"]