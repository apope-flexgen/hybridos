// flexservice is a library designed as singleton class
// which only returns an instance of that class
package flexservice

import (
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/flexgen-power/go_flexgen/fileops"
	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/shirou/gopsutil/process"
)

type flexService struct {
	procName     string
	realProcName string
	sockAddr     string
	apiMap       map[string]ApiCommand //track all apis registered
	versionStr   string
}

type ApiCommand struct {
	ApiName     string   //name of api - should be unique for service
	ApiDesc     string   //one line description of api
	ApiCallback Callback //callback function for the API
}

//constant values used by service
const pidFileDir string = "/var/run/flexgen"

// Defines callback function so it can be stored in the ApiCommand struct
// All Callback functions have the same input and output types
// Input: array of interface{} objects that can be converted into any datatype by the callback function
// Output: string that is sent back to the flexctl user over the socket connection
// because of the design of the Callback type, each implementation should be aware of how many args it expects, as well
// as the type and relevance of each arg in the input array
type Callback func(args []interface{}) (response string, err error)

var service *flexService = nil

// GetService returns the only pointer available for the service (singleton)
func GetService() *flexService {
	if service != nil {
		return service
	}
	var tmp flexService
	service = &tmp
	return service
}

// On successful connection, starts unix socket based server on the process name
// unique to every process instantiated by flexService
func (serv *flexService) startSock() (net.Listener, error) {
	sockName := serv.procName + "-service.sock"
	serv.sockAddr = filepath.Join("/tmp", sockName)
	//remove existing socket and create new one
	err := os.RemoveAll(serv.sockAddr)
	if err != nil {
		return nil, fmt.Errorf("failed to remove existing socket %s: %w", serv.sockAddr, err)
	}
	//start unix server
	log.MsgInfo("Creating Unix server...")
	l, err := net.Listen("unix", serv.sockAddr)
	if err != nil {
		return nil, fmt.Errorf("failed to listen to UNIX socket %s: %w", serv.sockAddr, err)
	}
	return l, nil
}

// Creates Unix socket based on process name and listens
// TODO: create sock name based on the process name
func (serv *flexService) Init(singleInstance bool, procName string) error {
	//Initialize necessary datastructures
	serv.apiMap = make(map[string]ApiCommand)
	//procname is what user wants to add. support for multiple instances
	procName = strings.TrimSpace(procName)
	if len(procName) > 0 {
		serv.procName = procName
	}
	//actual process name or binary name
	serv.realProcName = filepath.Base(os.Args[0])
	if singleInstance {
		//setup service to be single instance
		serv.setupSingleInstance()
	}
	//on success net.Listener is returned. Failure crashes program
	li, err := serv.startSock()
	if err != nil {
		return fmt.Errorf("error starting socket: %w", err)
	}
	err = serv.registerStdServices()
	if err != nil {
		return fmt.Errorf("standard api registration failed %w", err)
	}
	//start server when all Initialization is done
	go serv.startServer(li)
	return nil
}

// This method configures to make sure only one instance of process is running.
func (serv *flexService) setupSingleInstance() {
	//checking pid file
	pidFileName := serv.procName + ".pid"
	pidFullName := filepath.Join(pidFileDir, pidFileName)
	var pidFileExist bool
	if !fileops.Exists(pidFileDir) {
		//create pid directory if not exist
		log.MsgInfo("pid directory doesnt exist, creating one")
		err := os.MkdirAll(pidFileDir, 0755)
		if err != nil {
			log.Fatalf("MkdirAll error %s", err.Error())
		}
		pidFileExist = false
	} else {
		log.Infof("searching for pid file - %s", pidFullName)
		//check if pid file exist
		pidFileExist = true
		if !fileops.Exists(pidFullName) {
			pidFileExist = false
		}
	}

	if !pidFileExist {
		log.MsgInfo("pid file doesnt exist. Creating one!")
		err := fileops.CreatePidFile(pidFullName)
		if err != nil {
			log.Fatalf("pid file creation err %s", err.Error())
		}
		return
	}

	//If here, then pidfile already exist
	//read the pid of file and check if that matches to current process name
	pidcontent, err := ioutil.ReadFile(pidFullName)
	if err != nil {
		log.Fatalf("file read error %s", err.Error())
	}
	pidstr := string(pidcontent)
	first_line := strings.TrimSpace(strings.Split(pidstr, "\n")[0])
	first_str := strings.Split(first_line, " ")[0]
	filePid, err := strconv.Atoi(first_str)
	if err != nil {
		log.MsgFatal("error parsing pid, delete this file and also any service that is running for a fresh start")
	}

	log.Infof("pid from file read as %d", filePid)
	proc, err := process.NewProcess(int32(filePid))
	if err != nil {
		//If that process doesnt exist we are the only instance and safe to continue
		log.Infof("Process with pid %d no longer exist. safe to continue", filePid)
		err = fileops.CreatePidFile(pidFullName)
		if err != nil {
			log.Fatalf("pid file creation err %s", err.Error())
		}
		return
	}
	//process exist, verify this process already running
	cmdline, err := proc.Cmdline()
	if err != nil {
		log.Fatalf("Error getting cmdline of process %s", err.Error())
	}
	cmdlineFile := filepath.Base(cmdline)
	procnameInfile := strings.TrimSpace(strings.Split(cmdlineFile, " ")[0])
	log.Infof("process detected as %s for pid %d", procnameInfile, filePid)
	if procnameInfile == serv.realProcName {
		//exit if the process name matches. Dont create same instance
		log.MsgFatal("process already running. Exiting!!")
	}
}

// This function pre-registers some standard apis
// that are supported as part of this library.
func (serv *flexService) registerStdServices() error {
	//register standard commands built into service
	logCmd := ApiCommand{
		ApiName:     "set-log-level",
		ApiDesc:     "configure loglevel and verbose",
		ApiCallback: Callback(log.SetLogLevel),
	}
	err := serv.RegisterApi(logCmd)
	if err != nil {
		return fmt.Errorf("failed to register loglevel command API: %w", err)
	}
	redundantRateCmd := ApiCommand{
		ApiName:     "set-redundant-rate",
		ApiDesc:     "configure Redundant Rate",
		ApiCallback: Callback(log.SetRedundantRate),
	}
	err = serv.RegisterApi(redundantRateCmd)
	if err != nil {
		return fmt.Errorf("failed to register redundantRate command API: %w", err)
	}
	clearRateCmd := ApiCommand{
		ApiName:     "set-clear-rate",
		ApiDesc:     "configure Clear Rate",
		ApiCallback: Callback(log.SetClearRate),
	}
	err = serv.RegisterApi(clearRateCmd)
	if err != nil {
		return fmt.Errorf("failed to register clearRate command API: %w", err)
	}
	listCmd := ApiCommand{
		ApiName:     "list-apis",
		ApiDesc:     "display all apis supported",
		ApiCallback: Callback(listApis),
	}
	err = serv.RegisterApi(listCmd)
	if err != nil {
		return fmt.Errorf("failed to register list-apis command API: %w", err)
	}
	versionCmd := ApiCommand{
		ApiName:     "version",
		ApiDesc:     "displays current version of FTD",
		ApiCallback: Callback(getVersion),
	}
	err = serv.RegisterApi(versionCmd)
	if err != nil {
		return fmt.Errorf("failed to register version command API: %w", err)
	}
	return nil
}

// registers API that needs to be serviced
func (serv *flexService) RegisterApi(cmd ApiCommand) error {
	//checks if the apiname already exist in map and return false
	_, exists := serv.apiMap[cmd.ApiName]
	if exists {
		return errors.New("ApiName already registered")
	}
	//otherwise update map with this struct
	serv.apiMap[cmd.ApiName] = cmd
	return nil
}

// registers API that needs to be serviced
func (serv *flexService) SetVersion(str string) {
	serv.versionStr = str
}

// this routine should be started as thread serving only one client at any point of time.
// the response sent will be of sort <err-code> <response-str>
// 0 - success
// 1 - Invalid request format
// 2 - Invalid API
// 3 - failed to execute API
func (serv *flexService) startServer(l net.Listener) {
	for {
		conn, err := l.Accept()
		if err != nil {
			log.Errorf("Error accepting connection to server: %s", err.Error())
			time.Sleep(1 * time.Second)
			continue
		}
		log.MsgInfo("Server accepted new connection")
		buf := make([]byte, 512)
		i, err := conn.Read(buf)
		if err != nil {
			log.Errorf("Error reading buffer: %s", err.Error())
			continue
		}

		data := string(buf[0:i])
		cmdContents := strings.Split(data, " ")
		if len(cmdContents) == 0 {
			log.MsgError("Invalid flexservice command received")
			_, err = conn.Write([]byte("1 Invalid request"))
			if err != nil {
				log.MsgError("Failed to write Invalid response")
			}
			continue
		}
		apiCmd := cmdContents[0]
		log.Infof("Calling api command %s", apiCmd)
		for it := 1; it < len(cmdContents); it++ {
			log.Infof("Arguments received: %s", cmdContents[it])
		}
		api, exist := serv.apiMap[apiCmd]
		if !exist {
			_, err = conn.Write([]byte("2 Invalid API"))
			if err != nil {
				log.Errorf("Writing client error: %s", err.Error())
			}
			log.MsgInfo("End of client connection")
			continue
		}

		// convert arguments to []interface{}
		cmdArgs := make([]interface{}, len(cmdContents)-1)
		for i, v := range cmdContents[1:] {
			cmdArgs[i] = v
		}

		//write response from execution of callback function
		respStr := ExecuteAPI(&api, cmdArgs)
		_, err = conn.Write([]byte(respStr))
		if err != nil {
			log.Errorf("API result write error: %s", err.Error())
		}
		log.MsgInfo("End of client connection")
	}
}

// Execute callback function defined by APIcommand struct
func ExecuteAPI(api *ApiCommand, args []interface{}) string {
	resp, err := api.ApiCallback(args)
	//ignore response string when there is error
	if err != nil {
		return err.Error() + "\n"
	}
	//return the response string
	return resp
}

//  Default registered API that lists all available APIs on the current socket
//  All api callback function should follow this signature
func listApis(args []interface{}) (string, error) {
	var retVal string
	serv := GetService()
	retVal = "ApiCommand\t-\tDescription\n"
	retVal += "======================================\n"
	for key, val := range serv.apiMap {
		retVal += key + "\t-\t" + val.ApiDesc + "\n"
	}
	return retVal, nil
}

// Default registered API that returns version string if set by caller
func getVersion(args []interface{}) (string, error) {
	var retVal string = "no-version"
	serv := GetService()
	if len(serv.versionStr) > 0 {
		retVal = serv.versionStr
	}
	return retVal, nil
}
