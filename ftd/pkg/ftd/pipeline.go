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
	msgQueue FimsMsgBufferManager

	// takes messages from the message queue and loads them into their
	// respective encoder, periodically passing encoders to the archiver
	Collator *MsgCollator

	// takes encoders from the collator and archives the encoded data
	// into a .tar.gz file
	archiver *MsgArchiver
}

// Runs a complete pipeline for listening to messages and archiving them.
// New routines are started using the given error group and context.
// Does not returns until all stages stop or the pipeline encounters a fatal error.
func (p *Pipeline) Run(group *errgroup.Group, groupContext context.Context) error {
	// fims messages first arrive on the fims channel in the listener, which adds them to the msgQueue to
	// await processing. filters out messages that must be processed separately/uniquely (i.e. COPS messages)
	log.MsgDebug("Starting listener")
	p.listener = NewFimsListener()
	err := p.listener.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start listener stage: %w", err)
	}

	// messages are stored in the msgQueue until the collator is free to process them
	p.msgQueue = NewFimsMsgBufferManager(1, p.listener.Out)

	// collator encodes each message into an encoder that manages the message's URI. after a given
	// "archive period", collator passes a batch of URI encoders to the archiver
	log.MsgDebug("Starting collator")
	p.Collator = NewCollator(GlobalConfig.ArchivePeriod, p.msgQueue.Out)
	err = p.Collator.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start encoder stage: %w", err)
	}

	// archiver receives batches of encoders and archives each one into a .tar.gz file
	log.MsgDebug("Starting archiver")
	p.archiver = NewMsgArchiver(p.Collator.Out)
	err = p.archiver.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start archiver stage: %w", err)
	}

	// block until all stages stop or we hit a fatal error
	err = group.Wait()
	if err != nil {
		return fmt.Errorf("pipeline encountered a fatal error: %w", err)
	}
	return nil
}
