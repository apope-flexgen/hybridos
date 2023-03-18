package main

import (
	"fmt"
	"net"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/crypto/ssh"
)

// A managed connection wraps a raw network connection and provides timeouts on the connection's operations.
// Note that the underlying connection is not directly used by reads and writes on the ssh pipes, instead
// the underlying connection will continuously try to read on a loop and timeout if it doesn't get data for enough time.
// Note the call to go readLoop() in the golang ssh source code:
// https://cs.opensource.google/go/x/crypto/+/master:ssh/handshake.go;drc=ebe92624d1428c68f92576e1d27cc65d62bc2f7e;bpv=1;bpt=1;l=125?gsn=newClientTransport&gs=kythe%3A%2F%2Fgo.googlesource.com%2Fcrypto%3Flang%3Dgo%3Fpath%3Dssh%23func%2520newClientTransport
type managedConn struct {
	net.Conn
	timeout time.Duration
}

// Read from the connection
func (c *managedConn) Read(b []byte) (int, error) {
	// set a deadline that will trigger if we don't get data for longer than the timeout,
	// deadline will be reset by a write
	err := c.Conn.SetDeadline(time.Now().Add(c.timeout))
	if err != nil {
		return 0, fmt.Errorf("unable to set the connection read deadline: %w", err)
	}
	n, err := c.Conn.Read(b)
	if err != nil {
		log.Debugf("Read error: %v", err)
	}
	return n, err
}

// Write to the connection
func (c *managedConn) Write(b []byte) (int, error) {
	// set a deadline that will trigger if the write takes longer than the timeout,
	// deadline will be reset by a read
	err := c.Conn.SetDeadline(time.Now().Add(c.timeout))
	if err != nil {
		return 0, fmt.Errorf("unable to set the connection write deadline: %w", err)
	}
	n, err := c.Conn.Write(b)
	if err != nil {
		log.Debugf("Write error: %v", err)
	}
	return n, err
}

// Starts an ssh client connection where the underlying connection is a managedConn using the given timeout on reads and writes.
func sshDialWithManagedConnection(network, addr string, config *ssh.ClientConfig, timeout time.Duration) (*ssh.Client, error) {
	conn, err := net.DialTimeout(network, addr, config.Timeout)
	if err != nil {
		return nil, fmt.Errorf("failed the network dial: %w", err)
	}
	mConn := &managedConn{Conn: conn, timeout: timeout}
	c, chans, reqs, err := ssh.NewClientConn(mConn, addr, config)
	if err != nil {
		return nil, fmt.Errorf("failed to create the ssh client connection: %w", err)
	}
	return ssh.NewClient(c, chans, reqs), nil
}
