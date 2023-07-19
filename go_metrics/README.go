Phil Wilshire
05-05-2023


What we had to do to get the go_metrics build working.


1/ tidy up the pkg dir to just contain go_metrics
2/run
    go get github.com/flexgen-power/go_flexgen@52f567a64
    go get github.com/flexgen-power/echo/pkg/config@830c511
   this allowed
go mod init github.com/flexgen-power/go_metrics
   followed by
go mod tidy
 to work to produce
go.sum
   There may have been a few other go get things 
go get github.com/buger/jsonparser v1.1.1
   I had to do but the two above were the wacky ones.
   Then I had to get Kyle to tag my branch of fims.
   And put that tag in to Jenkinsfile.
   //Does the image need a special tag?
    imageTag="devel-11.1.0-fims-3.0.5.alpha.2"
add

mkdir -p $build_output
in the function build() in build_utils.sh


and voila it builds !!!!!!