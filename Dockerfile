FROM flexgen/centos7:release

# install rpms
ARG verNum
RUN yum install -y ess_controller_meta-$verNum

# copy over scripts
COPY ./scripts/ /home/scripts/
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