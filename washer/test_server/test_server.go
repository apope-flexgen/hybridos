package main

import (
	"bufio"
	"crypto/tls"
	"crypto/x509"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	"strings"
)

var batchesSince []byte
var batchInstructions []byte

func scanner(input chan<- string) {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		input <- scanner.Text()
	}
}

func readClient(conn net.Conn, clientInput chan<- string) {
	if conn == nil {
		log.Println("Socket reader received invalid connection")
		return
	}
	for {
		buffer := make([]byte, 1024)
		readbytes, err := conn.Read(buffer)
		if err == io.EOF {
			log.Println("client connection closed")
			return
		} else if err != nil || readbytes == 0 {
			log.Println("server failed to read:", err)
			return
		}
		log.Println("Read ", readbytes, " bytes while reading")
		clientInput <- string(buffer)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()

	input := make(chan string)
	go scanner(input)
	clientInput := make(chan string)
	go readClient(conn, clientInput)

	for {
		select {
		case clientInput := <-clientInput:
			log.Printf("recv: %s", string(clientInput))
			if strings.Contains(clientInput, "GetDispatchBatchesSince") {
				writebytes, err := conn.Write(batchesSince)
				if err != nil || writebytes == 0 {
					log.Println("server failed to write batchesSince")
					return
				}
				log.Println("wrote batchesSince successfully: ", writebytes)
			} else if strings.Contains(clientInput, "GetDispatchBatch") {
				writebytes, err := conn.Write(batchInstructions)
				if err != nil || writebytes == 0 {
					log.Println("server failed to write batchInstructions")
					return
				}
				log.Println("wrote batchInstructions successfully: ", writebytes)
			}
		case userInput := <-input:
			writebytes, err := conn.Write([]byte(userInput))
			if err != nil {
				log.Println("server failed to write:", err)
				return
			}
			if writebytes == 0 {
				log.Println("wrote 0 bytes")
				return
			}
		}
	}
}

func main() {
	var err error
	batchesSince, err = ioutil.ReadFile("/home/vagrant/git/washer/test_data/batchesSince.wsdl")
	if err != nil {
		log.Println("failed to read batchesSince file")
		return
	}

	batchInstructions, err = ioutil.ReadFile("/home/vagrant/git/washer/test_data/batchInstructions.wsdl")
	if err != nil {
		log.Println("failed to read batchInstructions file")
		return
	}

	caCertPEM, err := ioutil.ReadFile("/etc/pki/ca-trust/source/anchors/ca.crt")
	if err != nil {
		log.Println("Failed to read cacrt", err)
		return
	}

	roots := x509.NewCertPool()
	ok := roots.AppendCertsFromPEM(caCertPEM)
	if !ok {
		log.Println("failed to parse root certificate")
		return
	}

	cer, err := tls.LoadX509KeyPair("/usr/local/etc/config/washer/my.crt", "/usr/local/etc/config/washer/my.key")
	if err != nil {
		log.Println(err)
		return
	}

	config := &tls.Config{
		Certificates: []tls.Certificate{cer},
		ClientAuth:   tls.RequireAndVerifyClientCert,
		ClientCAs:    roots,
	}
	ln, err := tls.Listen("tcp", ":443", config)
	if err != nil {
		log.Println(err)
		return
	}
	defer ln.Close()

	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Println(err)
			continue
		}
		go handleConnection(conn)
	}
}
