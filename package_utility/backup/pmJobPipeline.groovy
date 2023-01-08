def call(body) {
    def config = [:]
    body.resolveStrategy = Closure.DELEGATE_FIRST
    body.delegate = config
    body()
	
	def gitAuthor
	def imageName = config.imageNames[0]
	def repoName = env.JOB_NAME.substring(env.JOB_NAME.indexOf("/")+1,env.JOB_NAME.lastIndexOf("/"))
	def TAG_NAME

	pipeline {
		agent { label 'master' }
		environment { ENVIRONMENT = "Jenkins"
		BUILD = "${env.BUILD_NUMBER}" }
		stages{
			stage('BUILD'){
				steps{
					sh "mkdir -p ${env.WORKSPACE}/build/release/"
					sh "./package_utility/build.sh -b"
				}
			}
			stage('PACKAGE'){
				steps{
					sh "./package_utility/package.sh"
				}
			}
			stage('PUSH'){
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
			stage('DOCKER IMAGE BUILD'){
				steps{
                    script{
						sh "./package_utility/docker.sh"
                    }
				}
			}
			stage('DOCKER CLEANUP'){
				steps{
					script{
						sh 'docker image prune -a -f'
					}
				}
			}
		}	
	}
}