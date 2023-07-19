package main

import (
	"fims"
	"log"

	"github.com/pkg/profile"
)

func main() {
	defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	f, err := fims.Connect("test")
	err = f.Subscribe("/components")
	recv_bufs := fims.Receiver_Bufs_Dynamic{}
	recv_bufs.Data_buf = make([]byte, f.GetMaxDatalen())

	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	for {
		f.ReceiveRawBufDynamic(&recv_bufs)
	}
}
