package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

type NetCollector struct {
	// configurable options
	DataMan DataManager `json:"collection"`
	Fims    bool
	Ports   bool
	IPs     map[string]string
	Stats   []string

	// internal vars
	ports  []string
	packet ICMPEchoPacket
	laddr  net.IPAddr
	buffer bytes.Buffer
}

type ICMPEchoPacket struct {
	Type        uint8
	Code        uint8
	Checksum    uint16
	Identifier  uint16
	SequenceNum uint16
}

// === Collector funcs ===

func (nett *NetCollector) init() error {
	if !nett.DataMan.Active {
		return fmt.Errorf("net is inactive")
	}
	if !nett.Fims && !nett.Ports && len(nett.IPs) == 0 && len(nett.Stats) == 0 {
		return fmt.Errorf("net is set to active but provides no stats to track")
	}

	contents, err := ioutil.ReadDir("/sys/class/net/")
	if err != nil {
		return fmt.Errorf("could not read /sys/class/net: %v", err)
	}

	// port setup
	nett.ports = make([]string, 0)
	for _, item := range contents {
		if strings.Contains(item.Name(), "eth") {
			nett.ports = append(nett.ports, item.Name())
		}
	}

	// ping setup
	nett.packet = ICMPEchoPacket{Type: 8}
	nett.laddr = net.IPAddr{IP: net.ParseIP("0.0.0.0")}

	binary.Write(&nett.buffer, binary.BigEndian, nett.packet)
	nett.packet.Checksum = computeChecksum(nett.buffer.Bytes())
	nett.buffer.Reset()
	binary.Write(&nett.buffer, binary.BigEndian, nett.packet)

	go nett.DataMan.start(nett)
	return nil
}

func (nett *NetCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "net",
	}

	if nett.Fims || len(nett.Stats) > 0 {
		data = mergeMaps(data, nett.getSockInfo())
	}
	if nett.Ports {
		data = mergeMaps(data, nett.getPortInfo())
	}
	if len(nett.IPs) > 0 {
		data = mergeMaps(data, nett.getConnectionInfo())
	}

	return data
}

// === NetCollector funcs ===
// these will all be run as goroutines within the Collector.scrape() func

func (nett *NetCollector) getSockInfo() map[string]interface{} {
	data := make(map[string]interface{})

	// fims socket info
	if nett.Fims {
		// read in the unix stats kept by the OS
		f, err := os.Open("/proc/net/unix")
		if err != nil {
			log.Errorf("could not read /proc/net/unix: %v", err)
			return data
		}
		defer f.Close()

		s := bufio.NewScanner(f)
		num_connections := 0
		for s.Scan() { // scan each line
			if strings.Contains(s.Text(), "FIMS") {
				fields := strings.Fields(s.Text()) // only want State, INode, Path
				if len(fields) < 8 {
					continue
				}
				if fields[3] == "00010000" { // listening socket
					status, err := strconv.ParseUint(fields[5], 0, 2)
					if err != nil {
						log.Errorf("could not resolve fims server status: %v", err)
					} else {
						data["fims_status"] = (int)(status)
					}
				} else if fields[3] == "00000000" { // connected socket
					num_connections++
				}
			}
		}
		data["fims_connections"] = num_connections
	}

	// additional global socket info
	for _, file := range nett.Stats {
		val, err := parseSoloUIntFile("/proc/sys/net/core/" + file)
		if err != nil {
			log.Errorf("problem parsing %s: %v", file, err)
		} else {
			data[file] = (int)(val)
		}
	}

	return data
}

func (nett *NetCollector) getPortInfo() map[string]interface{} {
	data := make(map[string]interface{})

	if nett.Ports {
		for _, port := range nett.ports {
			f, err := os.Open("/sys/class/net/" + port + "/operstate")
			if err != nil {
				log.Errorf("could not read dir %s: %v", "/sys/class/net/"+port+"/operstate", err)
			}
			defer f.Close()

			s := bufio.NewScanner(f)
			if s.Scan() {
				switch s.Text() {
				case "up":
					data[port+"_status"] = 1
				case "down":
					data[port+"_status"] = -1
				default:
					data[port+"_status"] = 404
				}
			} else {
				log.Errorf("could not open operstate for etho0")
			}
		}
	}

	return data
}

func (nett *NetCollector) getConnectionInfo() map[string]interface{} {
	data := make(map[string]interface{})

	wg := sync.WaitGroup{}
	mutex := sync.Mutex{}
	for name, ip := range nett.IPs {
		wg.Add(1)
		go func(name, ip string, data *map[string]interface{}, wg *sync.WaitGroup, mutex *sync.Mutex) {
			defer wg.Done()

			result, err := nett.ping(name, ip)
			if err != nil {
				log.Debugf("%v", err)
			}

			mutex.Lock()
			(*data)[name] = result
			mutex.Unlock()
		}(name, ip, &data, &wg, &mutex)
	}

	wg.Wait()
	return data
}

// === Helper funcs ===

func (nett *NetCollector) ping(name, ip string) (int, error) {
	raddr, err := net.ResolveIPAddr("ip", ip)
	if err != nil {
		return -1, fmt.Errorf("invalid ip: " + ip + "\n" + err.Error())
	}
	conn, err := net.DialIP("ip4:icmp", &nett.laddr, raddr)
	if err != nil {
		return -1, fmt.Errorf("dial error: %v", err)
	}
	defer conn.Close()

	if _, err := conn.Write(nett.buffer.Bytes()); err != nil {
		return -1, fmt.Errorf("write error: %v", err)
	}

	conn.SetReadDeadline(time.Now().Add(time.Second))

	recv := make([]byte, 1024)
	_, err = conn.Read(recv)
	if err != nil {
		return -1, fmt.Errorf("timeout: %v", err)
	}

	if computeChecksum(recv) != 0 {
		return -1, fmt.Errorf("checksum mismatch")
	}

	return 1, nil
}

func computeChecksum(data []byte) uint16 {
	var (
		sum    uint32
		length int = len(data)
		cur    int
	)
	// Add each 16 bits integer to sum
	for length > 1 {
		sum += uint32(data[cur])<<8 + uint32(data[cur+1])
		cur += 2
		length -= 2
	}
	// If it's odd, add remaining byte
	if length > 0 {
		sum += uint32(data[cur])
	}
	// Add the high 16 bits to the low 16 bits
	for sum>>16 != 0 {
		sum = (sum & 0xffff) + (sum >> 16)
	}

	return uint16(^sum)
}
