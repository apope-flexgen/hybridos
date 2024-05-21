# This Dockerfile builds the Docker images for Coordinate, Site Controller, ESS Controller, and PSM.
# These images are used for testing in virtual environments.
# Be aware that there are other Dockerfiles in the monorepo for building other images not listed above.

# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum makecache
RUN yum clean all

RUN yum install -y influxdb2-2.7.5
RUN yum install -y mongodb-org-6.0.14 mongodb-org-database-6.0.14 mongodb-org-database-tools-extra-6.0.14 mongodb-org-mongos-6.0.14 mongodb-org-server-6.0.14 mongodb-org-tools-6.0.14
RUN yum install -y $productName-$verNum
# do not edit this <<<

# copy over scripts
COPY ./bootstrap/ /home/scripts/
WORKDIR /home/scripts/
RUN chmod +x *.sh
WORKDIR /home/

# install bootstrap dependencies
RUN pip3 install --upgrade setuptools
RUN python3 -m pip install -U pip
RUN python3 -m pip install -r /home/scripts/requirements.txt

# expose application ports
# web_server
EXPOSE 443
# influxdb
EXPOSE 8086
# mongodb
EXPOSE 27017

# point to entrypoint script
ENTRYPOINT ["/home/scripts/run.sh"]