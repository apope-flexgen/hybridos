# This Dockerfile builds the Docker image for DTS.

# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum clean all
# do not edit this <<<

RUN mkdir -p /home/staging/
RUN mkdir -p /home/config/
RUN mkdir -p /home/data/
RUN mkdir -p /home/.error/
RUN mkdir -p /var/log/flexgen/dts/
RUN mkdir -p /var/lib/influxdb/
RUN mkdir -p /var/lib/mongo/

# Install DTS at a specific version.
RUN yum install -y dts-$verNum

# Copy the default dts.json into the image so it can be used for demos in Virtual Site.
# Virtual Site can override it with a custom dts.json via a bind-mount to /home/staging/dts.json.
# In production, mounting a production dts.json to /home/config/dts.json will cause the staged dts.json to be ignored.
COPY ./config/dts.json /home/staging/dts.json

COPY ./scripts /home/scripts
RUN chmod +x /home/scripts/*

ENTRYPOINT ["/home/scripts/run.sh"]