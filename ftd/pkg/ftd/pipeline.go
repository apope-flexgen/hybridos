package ftd

import (
	"context"
	"fmt"

	"golang.org/x/sync/errgroup"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

type Pipeline struct {
	// listens for messages to come through its FIMS connection and adds
	// them to the message queue, filtering out messages that are to be
	// handled uniquely (i.e. COPS messages)
	listener *FimsListener

	// message queue handles storing messages that need to be processed
	// while collator is busy so that listener does not get blocked
	msgQueue *FimsMsgBufferManager

	// message distributor distributes messsages across different data lanes
	distributor *MsgDistributor

	// loads messages into their respective encoders,
	// periodically passing encoders to the archiver
	Collators []*MsgCollator

	// takes encoders from the collator and archives the encoded data
	// into data files
	archivers []*MsgArchiver

	// takes data files from archiver and batches them into a
	// single compressed file per batch
	batchers []*ArchiveBatcher
}

// Runs a complete pipeline for listening to messages and archiving them.
// New routines are started using the given error group and context.
// Does not returns until all stages stop or the pipeline encounters a fatal error.
func (p *Pipeline) Run(cfg Config, group *errgroup.Group, groupContext context.Context) error {
	// fims messages first arrive on the fims channel in the listener, which adds them to the msgQueue to
	// await processing. filters out messages that must be processed separately/uniquely (i.e. COPS messages)
	log.MsgDebug("Starting listener")
	laneConfigsArray := []*LaneConfig{cfg.Lane1, cfg.Lane2, cfg.Lane3, cfg.Lane4, cfg.Lane5}
	aggregateUrisList := []UriConfig{} // aggregate list of all uri configs across all lanes
	for _, lane := range laneConfigsArray {
		if lane != nil {
			aggregateUrisList = append(aggregateUrisList, lane.Uris...)
		}
	}
	p.listener = NewFimsListener(aggregateUrisList)
	err := p.listener.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start listener stage: %w", err)
	}

	// messages are stored in the msgQueue until the collator is free to process them
	log.MsgDebug("Starting buffer manager")
	p.msgQueue = NewFimsMsgBufferManager(1, p.listener.Out)

	log.MsgDebug("Starting distributor")
	laneUrisLists := make([][]UriConfig, len(laneConfigsArray)) // list of uris lists for each lane, nil element indicates lane does not exist
	for i, lane := range laneConfigsArray {
		if lane != nil {
			laneUrisLists[i] = lane.Uris
		} else {
			laneUrisLists[i] = nil
		}
	}
	p.distributor = NewDistributor(laneUrisLists, p.msgQueue.Out, len(laneConfigsArray))

	p.Collators = make([]*MsgCollator, len(laneConfigsArray))
	p.archivers = make([]*MsgArchiver, len(laneConfigsArray))
	p.batchers = make([]*ArchiveBatcher, len(laneConfigsArray))
	for i, lane := range laneConfigsArray {
		if lane == nil {
			continue
		}

		p.distributor.NewOut(i)

		// collator encodes each message into an encoder that manages the message's URI. after a given
		// "archive period", collator passes a batch of URI encoders to the archiver
		log.Debugf("Starting collator for lane %d", i+1)
		p.Collators[i] = NewCollator(*lane, fmt.Sprint(i+1), p.distributor.Outs[i])
		err = p.Collators[i].Start(group, groupContext)
		if err != nil {
			return fmt.Errorf("failed to start encoder stage: %w", err)
		}

		// archiver receives batches of encoders and archives each one into a data file
		log.Debugf("Starting archiver for lane %d", i+1)
		p.archivers[i] = NewMsgArchiver(*lane, fmt.Sprint(i+1), p.Collators[i].Out)
		err = p.archivers[i].Start(group, groupContext)
		if err != nil {
			return fmt.Errorf("failed to start archiver stage: %w", err)
		}

		// batcher receives batches of archives and creates a single file per batch
		log.Debugf("Starting batcher for lane %d", i+1)
		p.batchers[i] = NewArchiveBatcher(cfg, *lane, fmt.Sprint(i+1), p.archivers[i].Out)
		err = p.batchers[i].Start(group, groupContext)
		if err != nil {
			return fmt.Errorf("failed to start batcher stage: %w", err)
		}
	}

	// start distributor once all of its outputs are connected
	err = p.distributor.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start distributor stage: %w", err)
	}

	// block until all stages stop or we hit a fatal error
	err = group.Wait()
	if err != nil {
		return fmt.Errorf("pipeline encountered a fatal error: %w", err)
	}
	return nil
}
