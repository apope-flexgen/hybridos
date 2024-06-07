package main

import (
	"encoding/json"
	"fims"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"time"
)

// Root config
type Config []MessageSenderConfig

// Config for messages on a set of uris
type MessageSenderConfig struct {
	PeriodMs            float64     `json:"period_ms"`            // publish period in milliseconds
	TemplatedDuplicates int         `json:"templated_duplicates"` // Number of times to duplicate across uris templated with # char
	Uri                 string      `json:"uri"`                  // uri string which may include a # template char
	MessageBody         interface{} `json:"message_body"`
}

// Runtime data for managing sending messages on a set of uris
type messageSender struct {
	uris               []string
	bodyBytes          []byte
	periodMicros       int64
	lastSendTimeMicros int64
}

func main() {
	// get config
	cfgFilePath := os.Args[1]
	cfgFile, err := os.Open(cfgFilePath)
	if err != nil {
		log.Fatalf("Failed to open config file: %s", err)
	}
	cfgBytes, err := io.ReadAll(cfgFile)
	if err != nil {
		log.Fatalf("Failed to read config file bytes: %s", err)
	}
	cfg := Config{}
	err = json.Unmarshal(cfgBytes, &cfg)
	if err != nil {
		log.Fatalf("Failed to unmarshal config: %s", err)
	}

	// connect to fims
	fimsObj, err := fims.Connect("fims_simulator")
	if err != nil {
		log.Fatalf("Failed to connect to fims: %s", err)
	}

	// setup message senders
	senders := []messageSender{}
	for _, senderConfig := range cfg {
		bodyBytes, _ := json.Marshal(senderConfig.MessageBody)
		uris := []string{}
		for i := 1; i <= senderConfig.TemplatedDuplicates; i++ {
			uris = append(uris, strings.ReplaceAll(senderConfig.Uri, "#", fmt.Sprint(i)))
		}
		senders = append(senders, messageSender{
			uris:               uris,
			bodyBytes:          bodyBytes,
			periodMicros:       int64(senderConfig.PeriodMs * 1000),
			lastSendTimeMicros: 0,
		})
	}

	// message sending loop
	for {
		for i := 0; i < len(senders); i++ {
			sender := &senders[i] // use ref so modifications are made to the stored sender data
			nowUnixMicro := time.Now().UnixMicro()
			sinceLastSendMicros := nowUnixMicro - sender.lastSendTimeMicros
			if sinceLastSendMicros < sender.periodMicros {
				continue
			}
			for _, uri := range sender.uris {
				fimsObj.SendRaw(fims.FimsMsgRaw{
					Method: "pub",
					Uri:    uri,
					Body:   sender.bodyBytes,
				})
			}
			// ensure sendtimes are always exactly on multiples of period so frequency is accurate
			sender.lastSendTimeMicros = nowUnixMicro - (nowUnixMicro % sender.periodMicros)
		}
		time.Sleep(time.Second / 30) // timing granularity is 1/30 of a second, modify as needed to support high rates
	}
}
