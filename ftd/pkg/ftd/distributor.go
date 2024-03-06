package ftd

import (
	"context"
	"fims"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Distributes any message received on input to the necessary outputs
type MsgDistributor struct {
	OutUriCfgs [][]UriConfig // List of uri configs lists for each output lane, nil element indicates no output lane
	in         <-chan *fims.FimsMsg
	Outs       []chan *fims.FimsMsg

	uriToOutsMemo map[string]outListMask // Memoized map from uri to out list used to speed up processing of repeated uris
}

// Bitmask over list of outputs indicating which ones to use.
// A bit of 1 indicates that an output should be used, a bit of 0 indicates the output should not be used.
// The least significant bit stands for the first output. The nth bit stands for the nth output.
type outListMask byte

// Creates a new message distributor with no output channels, but capacity for the given number of outputs
func NewDistributor(outUriCfgs [][]UriConfig, inputChannel <-chan *fims.FimsMsg, numOutputs int) *MsgDistributor {
	return &MsgDistributor{
		OutUriCfgs: outUriCfgs,
		in:         inputChannel,
		Outs:       make([]chan *fims.FimsMsg, numOutputs),

		uriToOutsMemo: map[string]outListMask{},
	}
}

// Creates a new output channel at the given index
func (distributor *MsgDistributor) NewOut(i int) {
	distributor.Outs[i] = make(chan *fims.FimsMsg, 1)
}

func (distributor *MsgDistributor) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	group.Go(func() error { return distributor.distributeUntil(groupContext.Done()) })
	return nil
}

// Loop for distributing messages from input to outputs
func (distributor *MsgDistributor) distributeUntil(done <-chan struct{}) error {
	defer func() {
		for _, out := range distributor.Outs {
			if out != nil {
				close(out)
			}
		}
	}()
	for {
		select {
		case <-done:
			goto termination
		case msg, ok := <-distributor.in:
			// handle channel close signal
			if !ok {
				goto termination
			}
			// distribute message to outputs
			distributor.distribute(msg)
		}
	}
termination:
	log.Infof("Distributor entered termination block. distributing remaining messages.")
	// graceful termination by distributing all remaining messages
	for msg := range distributor.in {
		distributor.distribute(msg)
	}
	log.Infof("Distributor terminating. All remaining messages were distributed.")
	return nil
}

// Distributes copies of the message to outputs.
// Concurrent modifications to the message body copies will be safe so long as those modifications only add new key-value pairs.
func (distributor *MsgDistributor) distribute(msg *fims.FimsMsg) {
	// verify message body is a map
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		log.Errorf("Message with URI %s is a %T, but map[string]interface{} is required", msg.Uri, msg.Body)
		return
	}

	outMask := distributor.getNecessaryOuts(msg.Uri)

	// distribute message to outputs based on bits in the necessary outs mask
	for i := 0; i < len(distributor.Outs); i++ {
		if outMask&(1<<i) != 0 {
			out := distributor.Outs[i]
			if out != nil {
				// copy message
				msgCopy := *msg
				// make a new shallow copy of the body so that other goroutines can safely add new key-value pairs to their copies
				bodyMapCopy := map[string]interface{}{}
				for key, val := range bodyMap {
					bodyMapCopy[key] = val
				}
				msgCopy.Body = bodyMapCopy

				out <- &msgCopy
			}
		}
	}
}

// Determines which outputs will need the given message based on its uri
func (distributor *MsgDistributor) getNecessaryOuts(msgUri string) outListMask {
	// check memo for preexisting entry
	if mask, ok := distributor.uriToOutsMemo[msgUri]; ok {
		return mask
	}
	// if no preexisting entry, check uris list of each output lane
	var mask outListMask = 0
	for i := 0; i < len(distributor.Outs); i++ {
		outUrisCfg := distributor.OutUriCfgs[i]
		if outUrisCfg != nil {
			// set the mask bit if the msg uri falls under the lane's config
			_, exists := findUriConfig(msgUri, outUrisCfg)
			if exists {
				mask |= (1 << i)
			}
		}
	}
	// update memo and return mask
	distributor.uriToOutsMemo[msgUri] = mask
	return outListMask(mask)
}
