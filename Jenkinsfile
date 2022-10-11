#!/usr/bin/env groovy

//See package_utility/README for a list of valid inputs

metaJobPipeline {

    //When would you like to trigger?
    buildTrigger="allCommits"

    //Which OS would you like to build on?
    imageNames=["centos7"]

    //Does the image need a special tag?
    imageTag="devel"

    //Run unit tests?
    unitTestStage=false

    //Run regression Tests?
    regTestStage=true

    //Which repository will RPMs be uploaded to?
    uploadRepo="flexgen"

    //Will a release of this repo need to trigger a docker image build?
    dockerBuild=false
}