
apope - 1/20/23


Information about Research Work for the Optimization of Fims Processing


The original goal was to use the power of goroutines to attempt to fully process 3000 Fims messages in 1 sec



Original Demo Video and Design Diagram
https://www.dropbox.com/home/Engineering/4%20-%20HybridOS/Demo/Ess%20Team/1-13-23



The main ideas and design are discussed in the folder above



In the design of the code, the use of a map or dictionary was decided on as optimal in order to be able to access various data that would be "uri-specific"

The use of the go map while also using a bunch of goroutines is frowned upon so it was a challenge to get it to work


In order to find the best option I have set up this package to be run to not only measure the speed of processing of fims messages in general using goroutines, but also I am comparing the speeds of 3 different types of maps


1. go built in map           (w/ sync.RWMutex)      		         https://go.dev/blog/maps

2. hashmap package           (thread/goroutine safe so no mutex)	 https://pkg.go.dev/github.com/cornelk/hashmap#Map

3. map in the sync package	 (thread/goroutine safe so no mutex)     https://pkg.go.dev/sync#Map




Package Run Steps

once in the echo/pkg/metrics_research/optimize_fims/maps_trial folder

1. go build

2. [go run *.go] || [go run -race *.go]







by doing the run command this is running all the files which are connected by one main method



Purpose of Each File

main.go
    where the main method and reused methods are
    At the top of main.go is "SIMS" and "NumFimsMsgs"
    You can change these global variables based on how many simulations and how many fims messages you want per simulation

phils_fims_stuff.go
    fims related work in go

publish_fims_msgs.go
    through different functions creates the ability to send random fims messages based on need

sim_go.map.go
    file that implements the "optimization of fims processing using goroutines" but with (built in go map w/ sync.RWMutex)

sim_hash_map.go
    file that implements the "optimization of fims processing using goroutines" but with (cornelk/hashmap pkg as used map)

sim_sync_map.go
    file that implements the "optimization of fims processing using goroutines" but with (sync.Map as used map)

csv_stuff.go
    few methods that are important for reading and writing data to and from csv files

results.csv
    where all the data from simulations is stored




As of 1/20/23
    The current standing is that the use of goroutines has proven to process the Fims Messages faster without the use of real hardware. The best map has not been decided yet. Once the map to move forward with for this project has been decided, the way to to take the project forward would be to just take the code within one of the "sim_...go" files that is specific to the map chosen. Take that code and strip it down to bare bones if desired; aka no time statistics or extra fluff; and run it against real hardware with real fims messages. 