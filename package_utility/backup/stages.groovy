def build() {
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

def unitTest() {
     sh "./../$newPath/test.sh"
}

def package() {
    sh "./../$newPath/package.sh"
}