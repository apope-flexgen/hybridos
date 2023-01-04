# do not edit this >>>
ARG imageName
ARG imageTag
FROM flexgen/$imageName:$imageTag

ARG productName
ARG dockerName
ARG verNum

RUN yum clean all
# do not edit this <<<

# install FIMS
RUN yum install -y fims-${verNum} fims-devel-${verNum}
