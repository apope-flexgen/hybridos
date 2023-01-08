def call(body) {
    def config = [:]
    body.resolveStrategy = Closure.DELEGATE_FIRST
    body.delegate = config
    body()

    def cloneStage=true
    def buildStage=true
    def packageStage=true
    def unitTestStage=false
    def regTestStage=true
    def pushStage=false

    def repoName = env.JOB_NAME.substring(env.JOB_NAME.indexOf("/")+1,env.JOB_NAME.lastIndexOf("/"))
    def imageName = config.imageName
    def imageTag = config.imageTag
    def uploadRepo = "flexgen-rpm"
    def uploadBucket = "snapshot"
    def gitAuthor
    def TAG_NAME

    // for docker repos, we build release* or devel* branches
    // pass env.BRANCH_NAME to docker image build downstream
    if((env.BRANCH_NAME.contains("release") || env.BRANCH_NAME.contains("devel")) && !env.BRANCH_NAME.contains("/")){
        pushStage=true
    }

    pipeline {
        agent{
            docker{
                alwaysPull true
                args '-v /etc/passwd:/etc/passwd -v /var/lib/jenkins/.ssh/:/var/lib/jenkins/.ssh/'
                image "flexgen/${imageName}:${imageTag}"
            }
        }
        environment{
            HOME = "${env.WORKSPACE}"
            GOPATH = "${env.WORKSPACE}/go"
            ENVIRONMENT = "Jenkins"
            BUILD = "${env.BUILD_NUMBER}"
        }
        stages{
            stage('DOCKER IMAGE BUILD'){
                when { allOf { expression { return fileExists('Dockerfile') }
                               expression { pushStage } } }
                steps{ // TODO: parallelize this step if possible
                    build job: 'Docker Image Build', parameters: [string(name: 'imageName', value: "${imageName}"), string(name: 'imageTag', value: "${imageTag}"), string(name: 'repoName', value: "${repoName}"), string(name: 'dockerTag', value: "${env.BRANCH_NAME}")]
                }
            }
        }
        post {
            failure {
                script{
                    if(buildStage){
                        slackSend channel: 'builds', color: '#D3D3D3', message: "INFO\njob:\t${env.JOB_NAME}\nbuild no:\t${env.BUILD_NUMBER}\nbranch:\t${env.BRANCH_NAME}\ntag:\t${TAG_NAME}\ncommit:\t${env.GIT_COMMIT[0..7]}\n(${env.BUILD_URL})"
                        slackSend channel: 'builds', color: '#FF0000', message: "FAILURE\nauthor:\t@${gitAuthor}"
                    }
                }
            }
            success {
                script{
                    if(pushStage){
                        slackSend channel: 'builds', color: '#D3D3D3', message: "INFO\njob:\t${env.JOB_NAME}\nbuild no:\t${env.BUILD_NUMBER}\nbranch:\t${env.BRANCH_NAME}\ntag:\t${TAG_NAME}\ncommit:\t${env.GIT_COMMIT[0..7]}\n(${env.BUILD_URL})"
                        slackSend channel: 'builds', color: '#00FF00', message: "SUCCESS"
                    }
                }
            }
        }
    }
}