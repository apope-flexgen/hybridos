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
    def imageName = config.imageNames[0] // TODO: support parallel multiple-OS builds
    def imageTag = config.imageTag
    def uploadRepo = "flexgen-rpm"
    def uploadBucket = "snapshot"
    def gitAuthor
    def TAG_NAME

    // if valid tag exists push artifacts
    if(env.TAG_NAME){
        // regex search the identified tag
        def pattern = ~/^(v{1})([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)(?:-(alpha|beta|rc|release))?(?:-(centos7|rocky8))?$/
        if (env.TAG_NAME ==~ pattern) {
            pushStage=true
        } else {
            error "invalid tag, aborting build"
        }
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
            stage('SETUP'){
                when { allOf { expression { cloneStage } } }
                steps{
                    sh "./git_checkout.sh -d ."
                }
            }
            stage('BUILD'){
                when { allOf { expression { buildStage } } }
                steps{
                    script{
                        gitAuthor = sh(script: "git --no-pager show -s --format='%ae' |  cut -f1 -d'@' | cut -f2 -d'+'", , returnStdout: true).trim()
                        TAG_NAME = sh(script: "git describe --match 'v*' --abbrev=0 --tags HEAD", , returnStdout: true).trim()
                    }
                    sh 'git config --global --add url."git@github.com:".insteadOf "https://github.com/"'
                    sh 'go env -w GO111MODULE=auto'

                    sh "./build.sh -b"
                }
            }
            stage('PACKAGE'){
                when { allOf { expression { packageStage } } }
                steps{
                    sh "./package.sh"
                }
            }
            stage('REGRESSION TEST'){
                when { allOf { expression { regTestStage } } }
                steps{
                    echo "skipping stage"
                }
            }
            stage('PUSH'){
                when { allOf { expression { pushStage } } }
                steps{
                    script{
                        def files = findFiles(glob: '**/output/*.rpm')
                        withCredentials([usernamePassword(credentialsId: 'artifactory-devops', passwordVariable: 'PWORD', usernameVariable: 'UNAME')]) {
                            for(int i = 0; i < files.size(); ++i){
                                if(files[i].name.contains('.alpha') || files[i].name.contains('.beta') || files[i].name.contains('.rc')){
                                    uploadBucket="snapshot"
                                } else {
                                    uploadBucket="release"
                                }
                                if(!files[i].name.contains('-devel') && !files[i].name.contains('-debuginfo')){
                                    sh "curl -X PUT -u ${UNAME}:${PWORD} --data-binary @${env.WORKSPACE}/${files[i].path} https://flexgenpower.jfrog.io/artifactory/${uploadRepo}/${imageName}/${uploadBucket}/${files[i].name}"
                                }
                            }
                        }
                    }
                }
            }
            stage('DOCKER IMAGE BUILD'){
                when { allOf { expression { return fileExists('Dockerfile') }
                               expression { pushStage } } }
                steps{ // TODO: parallelize this step if possible
                    build job: 'Docker Image Build', parameters: [string(name: 'imageName', value: "${imageName}"), string(name: 'imageTag', value: "${imageTag}"), string(name: 'repoName', value: "${repoName}"), string(name: 'dockerTag', value: "${env.TAG_NAME}")]
                }
            }
            stage('CLEANUP'){
                steps{
                    cleanWs()
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