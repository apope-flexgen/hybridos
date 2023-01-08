def call(body) {
    def config = [:]
    body.resolveStrategy = Closure.DELEGATE_FIRST
    body.delegate = config
    body()

    def prEvent=false
    def buildStage=false
    def packageStage=false
    def pushStage=false
    def gitAuthor
    def imageName = config.imageNames[0]
    def repoName = env.JOB_NAME.substring(env.JOB_NAME.indexOf("/")+1,env.JOB_NAME.lastIndexOf("/"))
    def infRebuild=false
    def TAG_NAME
    def newPath
    def successMessage=false;

    //If a tag is present which only happens on tag events build/test/package/push
    if(env.TAG_NAME){
        buildStage=true
        packageStage=true
        pushStage=true
        successMessage=true;
    }

    //If this is a pull request, build/test/package
    if(currentBuild.buildCauses.contains('Pull request ') || env.BRANCH_NAME.contains('PR-') || config.buildTrigger == 'allCommits'){ 
        buildStage=true
        packageStage=true}

    //If dev or release/* branch, build/test
    if(env.BRANCH_NAME == 'dev' || env.BRANCH_NAME.contains('release/')){
        buildStage=true}

    //If fims, do an infrastructure rebuild
    if(env.JOB_NAME.contains('FlexGen/fims/v')){
        infRebuild=true
    }

    if(config.imageTag.contains('base')){
        config.imageTag='latest'
    }else if(config.imageTag.contains('core')){
        config.imageTag='devel'
    }

    pipeline {
        agent {
            docker {
                alwaysPull true
                args '-v /etc/passwd:/etc/passwd -v /var/lib/jenkins/.ssh/:/var/lib/jenkins/.ssh/'
                image "flexgen/${imageName}:${config.imageTag}"
            }
        }
        environment { HOME = "${env.WORKSPACE}"
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
                        def functionsScript = libraryResource 'functions.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/functions.sh", text: functionsScript)
                        def versionScript = libraryResource 'version.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/version.sh", text: versionScript)
                        def packageScript = libraryResource 'package.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/package.sh", text: packageScript)
                        def dockerScript = libraryResource 'docker.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/docker.sh", text: dockerScript)
                        def testScript = libraryResource 'test.sh'
                        writeFile(file: "${env.WORKSPACE_TMP}/test.sh", text: testScript)

                        String path = "${env.WORKSPACE_TMP}"
                        String[] str;
                        str = path.split('/');
                        str.size();
                        newPath = str[str.size() - 1]
                    }
                    sh "chmod +x ../$newPath/build.sh"
                    sh "chmod +x ../$newPath/package.sh"
                    sh "chmod +x ../$newPath/test.sh"
                }
            }
            stage('BUILD'){
                when { expression { buildStage } }
                steps{
                    script{
                        gitAuthor = sh(script: "git --no-pager show -s --format='%ae' |  cut -f1 -d'@' | cut -f2 -d'+'", , returnStdout: true).trim()
                        TAG_NAME = sh(script: "git describe --match 'v*' --abbrev=0 --tags HEAD", , returnStdout: true).trim()
                    }
                    sh 'git config --global --add url."git@github.com:".insteadOf "https://github.com/"'
                    sh 'go env -w GO111MODULE=auto'

                    sshagent(credentials: ['github-devops-flexgen']) {
                        sh "./../$newPath/build.sh -b"
                    }
                }
            }
            stage('UNIT TEST'){
                when { allOf { expression { config.unitTestStage }
                            expression { buildStage } } }
                steps{
                    sh "./../$newPath/test.sh"
                }
            }
            stage('PACKAGE'){
                when { allOf { expression { packageStage }
                            expression { buildStage } } }
                steps{
                    sh "./../$newPath/package.sh"
                }
            }
            stage('PUSH'){
                when { allOf { expression { pushStage }
                            expression { buildStage }} }
                steps{
                    script{
                        def files = findFiles(glob: '**/rpmbuild/RPMS/x86_64/*')
                        withCredentials([usernamePassword(credentialsId: 'artifactory-devops', passwordVariable: 'PWORD', usernameVariable: 'UNAME')]) {
                            for(int i = 0; i < files.size(); ++i){
                                if(files[i].name.contains('-devel') || files[i].name.contains('.rc.')){
                                    sh "curl -X PUT -u ${UNAME}:${PWORD} --data-binary @${env.WORKSPACE}/${files[i].path} https://flexgenpower.jfrog.io/artifactory/flexgen-rpm/${imageName}/devel/${files[i].name}"
                                }else if(!files[i].name.contains('-debuginfo')){
                                    sh "curl -X PUT -u ${UNAME}:${PWORD} --data-binary @${env.WORKSPACE}/${files[i].path} https://flexgenpower.jfrog.io/artifactory/flexgen-rpm/${imageName}/release/${files[i].name}"
                                }
                            }
                        }
                    }
                }
            }
            stage('DOCKER IMAGE REBUILD'){
                when { allOf { expression { buildStage }
                               expression { config.dockerBuild } } }
                steps{
                    build job: 'Docker Image Build',parameters: [string(name: 'repoName', value: "${repoName}"), string(name: 'dockerImageName', value: "${config.dockerImageName}"), string(name: 'dockerTag', value: "${config.dockerTag}") ]
                }
            }
            stage('Trigger Infrastructure Build'){
                when { allOf {  expression { infRebuild }
                                expression { pushStage }
                                expression { buildStage } } }
                steps{
                    build job: 'Infrastructure Build',parameters: [string(name: 'fimsVer', value: "${env.TAG_NAME}".drop(1))]
                }
            }
        }
        post {
            success {
                script{
                    if(successMessage){
                        slackSend channel: 'builds', color: '#D3D3D3', message: "INFO\njob:\t${env.JOB_NAME}\nbuild no:\t${env.BUILD_NUMBER}\nbranch:\t${env.BRANCH_NAME}\ntag:\t${TAG_NAME}\ncommit:\t${env.GIT_COMMIT[0..7]}\n(${env.BUILD_URL})"
                        slackSend channel: 'builds', color: '#00FF00', message: "SUCCESS"
                    }
                }
            }
        }
    }
}
