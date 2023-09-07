FROM centos:centos7 as base

RUN yum -y update

RUN yum install -y epel-release
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-7-gcc*
RUN yum install -y make
RUN yum install -y vim

RUN yum install -y nodejs-16.18.1
RUN yum install -y npm-8.19.2
RUN yum install -y python3

WORKDIR /usr/src/app

COPY package.json /usr/src/app/package.json
COPY package-lock.json /usr/src/app/package-lock.json

FROM base as test
SHELL [ "/usr/bin/scl", "enable", "devtoolset-7"]
RUN npm install
COPY . /usr/src/app
RUN npm config set python python3
RUN npm run test:docker

FROM base as dev
SHELL [ "/usr/bin/scl", "enable", "devtoolset-7"]
RUN npm install
COPY . /usr/src/app
RUN npm config set python python3
CMD ["npm", "run", "start:docker"]
