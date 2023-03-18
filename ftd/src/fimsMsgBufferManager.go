// TODO : At some point it would be nice to use the buffman provided in go_flexgen, but we probably need generics before then

package main

import "fims"

// Adapted from go_flexgen buffman

// Contains a slice that can have new elements pushed onto the end of it by sending them
// through the In channel. Elements can be popped off the front of the queue by reading
// them from the Out channel. It will try to add input data to its queue as fast as it can,
// so sends to the In channel should practically never block.
type fimsMsgBufferManager struct {
	in    <-chan *fims.FimsMsg
	out   chan *fims.FimsMsg
	queue []*fims.FimsMsg
}

// Instantiate a new BufferManager where the Out channel has a buffer
// matching the number of consumers that are expected to read from it.
func newFimsMsgBufferManager(numConsumers int, inputChannel <-chan *fims.FimsMsg) fimsMsgBufferManager {
	ch := fimsMsgBufferManager{
		in:    inputChannel,
		out:   make(chan *fims.FimsMsg, numConsumers),
		queue: make([]*fims.FimsMsg, 0),
	}

	ch.open()

	return ch
}

// Opens up the buffer for use by starting a routine that manages the buffer.
func (buffMan *fimsMsgBufferManager) open() {
	go func() {
		for {
			if len(buffMan.queue) == 0 {
				// if the queue is empty, cannot feed any elements through the Out
				// channel so block on waiting for new elements
				in := <-buffMan.in
				buffMan.queue = append(buffMan.queue, in)
			} else {
				// if the queue is not empty, block on waiting for new elements or
				// feeding elements to consumers
				select {
				case in := <-buffMan.in:
					buffMan.queue = append(buffMan.queue, in)
				case buffMan.out <- buffMan.queue[0]:
					buffMan.queue = buffMan.queue[1:]
				}
			}
		}
	}()
}
