package main

import (
	"bufio"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"time"
)

func main() {
	var clear bool
	flag.BoolVar(&clear, "c", false, "include to clear directory and wait")
	var instant bool
	flag.BoolVar(&instant, "i", false, "include to copy files instantly")
	var fileCt int
	flag.IntVar(&fileCt, "f", -1, "specify number of files to be transferred")
	var dir string 
	flag.StringVar(&dir, "d", "/home/vagrant/site_data", "specify directory to copy files from")
	flag.Parse()

	if clear {
		err := os.RemoveAll("/home/vagrant/data/")
		if err != nil {
			log.Fatal(err)
		}
		log.Println("[CLEAN] successfully removed ~/data")
		
		os.MkdirAll("/home/vagrant/data/", 0775)
		if err != nil {
			log.Fatal(err)
		}
		log.Println("[CLEAN] successfully recreated ~/data")

		reader := bufio.NewReader(os.Stdin)
		fmt.Print("PRESS ENTER ->")
		reader.ReadString('\n')
		fmt.Println("CONTINUING...")
	}
	
	files, err := ioutil.ReadDir(dir)
	if err != nil {
		log.Fatal(err)
	}

	for i, f := range files {
		cmd := exec.Command("sudo", "cp", dir + "/" + f.Name(), "/home/vagrant/data/" + f.Name())
		cmd.Run()
		log.Println("[COPY] " + f.Name())
		if i == fileCt-1 {
			break
		}
		if !instant {
			time.Sleep(time.Second/2)
		}
	}
}