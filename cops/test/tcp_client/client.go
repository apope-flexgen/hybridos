// Basic TCP client with which to test C2C communication interface
package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"strconv"
	"strings"
)

var startWord = "C2C"

func main() {
	conn := connectToServer()
	go promptMessage(conn)
	go receiveMessage(conn)
	for {
	}
}

func connectToServer() net.Conn {
	conn, err := net.Dial("tcp", "127.0.0.1:8000")
	if err != nil {
		panic("Dial to server failed")
	}
	fmt.Println("Message start-word is", startWord)
	return conn
}

func promptMessage(conn net.Conn) {
	terminalReader := bufio.NewReader(os.Stdin)
	fmt.Print("Text to send: ")
	msg, err := terminalReader.ReadString('\n')
	if err != nil {
		panic("Failed to read user input")
	}
	sendTCPMsg(conn, strings.Trim(msg, "\n"))
}

func receiveMessage(conn net.Conn) {
	for {
		reader := bufio.NewReader(conn)
		startOfMsg, err := reader.Peek(len(startWord))
		if err != nil {
			panic("Server connection lost!")
		} else if string(startOfMsg) != startWord {
			reader.Discard(1)
			failureMsg := fmt.Sprint("Error: message started with ", string(startOfMsg), " not ", startWord, "\n")
			fmt.Println(failureMsg)
			sendTCPMsg(conn, failureMsg)
			continue
		}
		reader.Discard(len(startWord))
		msgSizeString, _ := reader.ReadString(' ')
		msgSize, _ := strconv.Atoi(strings.Trim(msgSizeString, " "))
		msgBuffer := make([]byte, msgSize)
		reader.Read(msgBuffer)
		msg := string(msgBuffer)
		fmt.Println("Message Received: ", msg)
		go promptMessage(conn)
	}
}

func sendTCPMsg(conn net.Conn, msgBody string) {
	msgHeader := startWord + strconv.Itoa(len(msgBody)) + " "
	fmt.Fprintf(conn, msgHeader+msgBody)
}
