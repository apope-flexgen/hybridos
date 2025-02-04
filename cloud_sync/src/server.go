package main

import (
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"
	"strings"
	"time"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/s3"
	"github.com/aws/aws-sdk-go/service/s3/s3manager"
	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"
	"golang.org/x/crypto/ssh/knownhosts"
)

// A server is a destination to which files must be transferred.
// It can be a remote server or a local server.
// The server struct contains any structs that might be needed to connect to the destination.
type server struct {
	name   string
	config ServerConfig

	// SSH components
	// maps client names to SSH client configs used if connecting over SSH
	sshConfigs map[string]*ssh.ClientConfig
	// maps client names to SSH clients used if connecting over SSH
	sshClients map[string]*ssh.Client

	// maps client names to SSH pipes needed when using SCP
	sshPipes map[string]*sshPipe

	// maps client names to SFTP clients which can be used on top of SSH components if SFTP is enabled
	sftpClients map[string]*sftp.Client

	// S3 uploader used if uploading to an S3 bucket
	s3Uploader *s3manager.Uploader
}

type ServerConfig struct {
	IP                  string
	Port                string
	User                string
	Dir                 string `json:"directory"`
	Sorted              bool
	SortedRetentionDays int `json:"sorted_retention_days"`
	Timeout             int

	// S3 components
	Bucket string
	Region string

	// Set true to enable SFTP usage
	UseSFTP bool `json:"use_sftp"`
}

type sshPipe struct {
	in  io.WriteCloser
	out io.Reader
}

type serverMap map[string]*server

var servers serverMap = make(serverMap)

// Unmarshal a server config
func (cfg *ServerConfig) UnmarshalJSON(data []byte) error {
	type MethodlessConfigAlias ServerConfig // type alias with no methods needed to prevent recursive calls to UnmarshalJSON

	// default values
	tmpCfg := &MethodlessConfigAlias{
		SortedRetentionDays: 365,
	}

	err := json.Unmarshal(data, tmpCfg)
	if err != nil {
		return err
	}

	*cfg = ServerConfig(*tmpCfg)
	return nil
}

// Creates a new server based on the provided name and config.
// Standardizes names to be all lower-case and have no spaces.
// If the server is remote, allocates memory to manage its SSH interface.
func createServer(name string, cfg ServerConfig) (*server, error) {
	newServer := &server{
		name:   strings.ReplaceAll(strings.ToLower(name), " ", "_"),
		config: cfg,
	}

	// if server is remote, we must have a positive timeout
	if cfg.IP != "" || cfg.Bucket != "" {
		if cfg.Timeout <= 0 {
			return nil, fmt.Errorf("configured timeout for remote server must be positive")
		}
	}

	// if S3 connection, do nothing yet
	// S3 does not use an IP either, so check this before assuming the configuration is local
	if cfg.Bucket != "" {
		return newServer, nil
	}

	// if server is local, ensure directory exists
	if cfg.IP == "" {
		err := ensureDirectoryExists(cfg.Dir)
		if err != nil {
			return nil, fmt.Errorf("unable to ensure local server directory exists: %w", err)
		}
		return newServer, nil
	}

	// if not local or S3, assume SSH >>>
	// only allocate memory for SSH connection if server is remote
	newServer.sshConfigs = make(map[string]*ssh.ClientConfig)
	newServer.sshClients = make(map[string]*ssh.Client)

	if cfg.UseSFTP {
		// if SFTP is used we also need to allocate memory for sftp clients
		newServer.sftpClients = make(map[string]*sftp.Client)
	} else {
		// otherwise SSH pipes are needed
		newServer.sshPipes = make(map[string]*sshPipe)
	}

	return newServer, nil
}

// Loads connection-specific configuration for the connection between a server and client.
// Currently only loads SSH config data.
func (serv *server) setupConnection(cl *client) error {
	// connection is SSH if not local and not S3
	if serv.config.IP != "" && serv.config.Bucket == "" {
		buf, err := os.ReadFile(cl.config.Key)
		if err != nil {
			return fmt.Errorf("failed to read private key file: %w", err)
		}

		signer, err := ssh.ParsePrivateKey(buf)
		if err != nil {
			return fmt.Errorf("failed to parse private key: %w", err)
		}

		knownHostsCallback, err := knownhosts.New(cl.config.Knownhosts)
		if err != nil {
			return fmt.Errorf("could not create knownhost callback: %w", err)
		}

		sshConf := ssh.ClientConfig{
			User:            serv.config.User,
			Auth:            []ssh.AuthMethod{ssh.PublicKeys(signer)},
			HostKeyCallback: knownHostsCallback,
			Timeout:         time.Second * time.Duration(serv.config.Timeout),
		}
		serv.sshConfigs[cl.name] = &sshConf
	}

	return nil
}

// initializes the connection from the client to the server
func (serv *server) initConnection(cl *client) error {
	// S3 configuration
	if serv.config.Bucket != "" {
		if serv.config.Timeout == 0 {
			return fmt.Errorf("timeout must be > 0")
		}

		return serv.createS3Uploader(cl.config.AWSId, cl.config.AWSSecret)
	}

	// Nothing to do if the server is local
	if serv.config.IP == "" {
		return nil
	}

	// if not local or S3, assume SSH >>>
	err := serv.createSSH(cl)
	if err != nil {
		return fmt.Errorf("could not create SSH tunnel from %s to %s: %w", cl.name, serv.name, err)
	}

	if serv.config.UseSFTP {
		// if using SFTP, we need to create an SFTP client on top of the SSH client
		sftpCl, err := serv.createSFTP(cl)
		if err != nil {
			return fmt.Errorf("could not create SFTP client from %s to %s: %w", cl.name, serv.name, err)
		}
		serv.sftpClients[cl.name] = sftpCl
	} else {
		// otherwise, direct access to SSH pipes is needed
		pipes, err := serv.createPipes(cl)
		if err != nil {
			return fmt.Errorf("could not create SSH pipe from %s to %s: %w", cl.name, serv.name, err)
		}
		serv.sshPipes[cl.name] = &pipes
	}

	return nil
}

// Dials the server using SSH and adds the new SSH client to the SSH client map under the given CloudSync client's name.
func (serv *server) createSSH(cl *client) error {
	if oldClient := serv.sshClients[cl.name]; oldClient != nil {
		err := oldClient.Close()
		if err != nil {
			// this log is debug severity since it is expected that an error occur when closing a closed client
			log.Debugf("Error closing SSH client for server %s from client %s: %v.", serv.name, cl.name, err)
		}
	}
	sshCl, err := sshDialWithManagedConnection("tcp", net.JoinHostPort(serv.config.IP, serv.config.Port), serv.sshConfigs[cl.name], time.Second*time.Duration(serv.config.Timeout))
	serv.sshClients[cl.name] = sshCl
	if err != nil {
		return fmt.Errorf("SSH dial failed: %w", err)
	}
	return nil
}

// Verifies there is a valid SSH client (creating one if not) and attempts to open
// a new SSH session. Will return a struct of the stdin, stdout, and stderr pipes
// for the session.
func (serv *server) createPipes(cl *client) (sshPipe, error) {
	if serv.sshClients[cl.name] == nil {
		err := serv.createSSH(cl)
		if err != nil {
			return sshPipe{}, fmt.Errorf("SSH client was non-existent and failed to create one: %w", err)
		}
	}

	session, err := serv.sshClients[cl.name].NewSession() // start new session
	if err != nil {
		return sshPipe{}, fmt.Errorf("failed to create new session: %w", err)
	}

	in, err := session.StdinPipe() // create session input pipe
	if err != nil {
		return sshPipe{}, fmt.Errorf("failed to create stdin pipe: %w", err)
	}

	out, err := session.StdoutPipe()
	if err != nil {
		return sshPipe{}, fmt.Errorf("failed to create stdout pipe: %w", err)
	}

	return sshPipe{
		in:  in,
		out: out,
	}, session.Shell()
}

// Closes any preexisting SFTP client and then creates a new SFTP client using the SSH client.
func (serv *server) createSFTP(cl *client) (*sftp.Client, error) {
	if serv.sshClients[cl.name] == nil {
		err := serv.createSSH(cl)
		if err != nil {
			return nil, fmt.Errorf("SSH client was non-existent and failed to create one: %w", err)
		}
	}

	if oldClient := serv.sftpClients[cl.name]; oldClient != nil {
		err := oldClient.Close()
		if err != nil {
			log.Debugf("Error closing SFTP client for server %s from client %s: %v.", serv.name, cl.name, err)
		}
	}
	sftpCl, err := sftp.NewClient(serv.sshClients[cl.name])
	serv.sftpClients[cl.name] = sftpCl
	if err != nil {
		return nil, fmt.Errorf("SFTP client creation failed: %w", err)
	}
	return sftpCl, nil
}

// Asynchronously repeatedly tries to reestablish the connection until the connection is back up.
// Returns a channel which is closed when the connection is back up.
func (serv *server) asyncReestablishConnection(cl *client) (connectionReestablished <-chan struct{}) {
	complete := make(chan struct{})
	go func() {
		for {
			reestablished := serv.reestablishConnection(cl)
			if reestablished {
				// signal that the connection has been reestablished and then return
				close(complete)
				return
			} else {
				time.Sleep(time.Second * time.Duration(config.SleepLimitSeconds))
			}
		}
	}()
	return complete
}

// run diagnostics to try to re-establish the client-server connection (remote servers) or recreate the server
// directory (local servers).
func (serv *server) reestablishConnection(cl *client) (reestablished bool) {
	log.Warnf("Connection from client %s to server %s is faulty. Rechecking connection status...", cl.name, serv.name)

	if serv.config.Bucket != "" { // s3
		err := serv.createS3Uploader(cl.config.AWSId, cl.config.AWSSecret)
		if err != nil {
			log.Errorf("Failed to create S3 uploader from client %s to server %s: %v", cl.name, serv.name, err)
			return false
		} else {
			log.Infof("Successfully recreated S3 uploader from client %s to server %s.", cl.name, serv.name)
		}
	} else if serv.config.IP != "" { // if server is remote, check SSH connection
		err := serv.checkConn(cl)
		if err != nil {
			log.Errorf("Connection from client %s to server %s is still faulty: %v. Attempting to recreate connection now...", cl.name, serv.name, err)
			err = serv.createSSH(cl)
			if err != nil {
				log.Errorf("Error creating SSH connection from client %s to server %s: %v", cl.name, serv.name, err)
				return false
			} else {
				log.Infof("Successfully recreated connection from client %s to server %s.", cl.name, serv.name)
			}
		}

		if serv.config.UseSFTP {
			// if using SFTP, we need to recreate the SFTP client on top of the SSH client
			sftpCl, err := serv.createSFTP(cl)
			if err != nil {
				log.Errorf("Could not create SFTP client from %s to %s: %v", cl.name, serv.name, err)
				return false
			}
			serv.sftpClients[cl.name] = sftpCl
		} else {
			// create fresh SSH pipes otherwise
			pipes, err := serv.createPipes(cl)
			if err != nil {
				log.Errorf("Error creating SSH pipe/session: %v.", err)
				return false
			}
			serv.sshPipes[cl.name] = &pipes
		}
	} else { // if server is local, check existence of server directory
		err := ensureDirectoryExists(serv.config.Dir)
		if err != nil {
			log.Errorf("ensureDir issue: %v", err)
			return false
		}
	}
	return true
}

// Verifies that there is still a non-nil SSH connections and that those connections work with a ping.
// If the connection is nil or the ping errors or times out, an error is returned signaling the caller
// that it needs to re-establish a connection.
func (serv *server) checkConn(cl *client) error {
	// verify client is not nil
	if serv.sshClients[cl.name] == nil {
		return fmt.Errorf("sshClient for server %s is nil", serv.name)
	}
	// verify client conn is not nil
	if serv.sshClients[cl.name].Conn == nil {
		return fmt.Errorf("sshClient.Conn for server %s is nil", serv.name)
	}

	// send a ping and if the ping completes, check its error code
	_, _, err := serv.sshClients[cl.name].Conn.SendRequest("ping", true, []byte{'1'})
	if err != nil {
		return fmt.Errorf("ping to server returned error: %w", err)
	}
	return nil
}

// Creates the uploader/session for S3 uploads.
func (serv *server) createS3Uploader(aws_id, secret string) error {
	// create aws session
	sess, err := session.NewSession(
		&aws.Config{
			Credentials:      credentials.NewStaticCredentials(aws_id, secret, ""), // no token
			Region:           aws.String(serv.config.Region),
			S3ForcePathStyle: aws.Bool(true),
		},
	)
	if err != nil {
		return fmt.Errorf("could not create session: %w", err)
	}

	serv.s3Uploader = s3manager.NewUploader(sess)

	// check that bucket can actually be accessed
	input := &s3.HeadBucketInput{
		Bucket: aws.String(serv.config.Bucket),
	}

	// HeadBucket will return an error if we cannot access the bucket
	_, err = serv.s3Uploader.S3.HeadBucket(input)
	if err != nil {
		return fmt.Errorf("error checking for S3 bucket: %w", err)
	}

	return nil
}
