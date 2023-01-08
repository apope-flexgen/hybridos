def call(body) {
    def config = [:]
    body.resolveStrategy = Closure.DELEGATE_FIRST
    body.delegate = config
    body()

    def buildStage=true
    def packageStage=true
    def unitTestStage=true
    def regTestStage=false
    def pushStage=false

    def repoName = env.JOB_NAME.substring(env.JOB_NAME.indexOf("/")+1,env.JOB_NAME.lastIndexOf("/"))
    def imageName = config.imageNames[0] // TODO: support parallel multiple-OS builds
    def imageTag = config.imageTag
    def uploadRepo = "flexgen-rpm"
    def uploadBucket = "snapshot"
    def gitAuthor
    def TAG_NAME
    def envPath

    // handle environment standardization (i.e. devel, devel-10.3.0, devel-10.3.0-fims-3.0.0)
    if(config.imageTag.contains('base')) {
        imageTag = 'latest'
    } else {
        imageTag = config.imageTag
    }
    
    // uploadRepo logic
    if((config.uploadRepo)  && (config.uploadRepo.contains('third_party') || config.uploadRepo.contains('third-party'))){
        uploadRepo = "third-party-rpm"
    } else {
        uploadRepo = "flexgen-rpm"
    }

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

    pipeline{
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
                steps{
                    script{
                        def buildScript = libraryResource 'build.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/build.sh", text: buildScript)

                        def packageScript = libraryResource 'package.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/package.sh", text: packageScript)
                        
                        def testScript = libraryResource 'test.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/test.sh", text: testScript)
                        
                        def dockerScript = libraryResource 'docker.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/docker.sh", text: dockerScript)

                        def versionScript = libraryResource 'version.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/version.sh", text: versionScript)

                        def functionsScript = libraryResource 'functions.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/functions.sh", text: functionsScript)

                        String path = "${env.WORKSPACE_TMP}"
                        String[] str;
                        str = path.split('/');
                        str.size();
                        envPath = str[str.size() - 1]
                    }
                    sh "chmod +x ../$envPath/build.sh"
                    sh "chmod +x ../$envPath/package.sh"
                    sh "chmod +x ../$envPath/test.sh"
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

                    sshagent(credentials: ['github-devops-flexgen']) {
                        sh "./../$envPath/build.sh -b"
                    }
                }
            }
            stage('UNIT TEST'){
                when { allOf { expression { unitTestStage } } }
                steps{
                    echo "skipping stage"
                }
            }
            stage('PACKAGE'){
                when { allOf { expression { packageStage } } }
                steps{
                    sh "./../$envPath/package.sh"
                }
            }
            stage('PUSH'){
                when { allOf { expression { pushStage } } }
                steps{
                    script{
                        def files = findFiles(glob: '**/rpmbuild/RPMS/x86_64/*.rpm')
                        withCredentials([usernamePassword(credentialsId: 'artifactory-devops', passwordVariable: 'PWORD', usernameVariable: 'UNAME')]) {
                            for(int i = 0; i < files.size(); ++i){
                                if(uploadRepo.contains("third-party-rpm")){ // issuing third_party package
                                    if(files[i].name.contains('.alpha') || files[i].name.contains('.beta') || files[i].name.contains('.rc')){
                                        uploadBucket = "snapshot"
                                    } else if(files[i].name.contains('-devel')){
                                        uploadBucket = "devel"
                                    } else {
                                        uploadBucket = "release"
                                    }
                                } else if(uploadRepo.contains("flexgen-rpm")){ // issuing component package
                                    if(files[i].name.contains('-devel')){
                                        uploadBucket = "devel"
                                    } else {
                                        uploadBucket = "snapshot"
                                    }
                                }
                                if(!files[i].name.contains('-debuginfo')){ // do not push debuginfo
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
                steps{
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
