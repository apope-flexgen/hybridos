// Package buffman implements a queue that can have elements added to it by
// inserting them through the in channel and can have elements removed from it by
// taking them from the out channel. The queue will expand and shrink in size as
// elements are added to it and removed from it with no upper limit.
//
// The basic use case for buffman is in a producer-consumer concurrency pattern
// where the producer is generating jobs based on events and needs to store those jobs
// and get back to processing events without waiting for a consumer to take the job.
package buffman

// Contains a slice that can have new elements pushed onto the end of it by sending them
// through the in channel. Elements can be popped off the front of the queue by reading
// them from the out channel. It will try to add input data to its queue as fast as it can,
// so sends to the in channel should practically never block.
type BufferManager struct {
	in         <-chan string
	out        chan string
	buffer     []string
	buffertype BufferType
}

// Queue (0) or Stack (1)
type BufferType int

const (
	Queue BufferType = 0
	Stack BufferType = 1
)

// Instantiate a new BufferManager where the out channel has a buffer
// matching the number of consumers that are expected to read from it.
// The out channel is allocated here, but the in channel should be
// supplied by the caller.
func New(buffertype BufferType, numConsumers int, inputChannel <-chan string) BufferManager {
	ch := BufferManager{
		in:         inputChannel,
		out:        make(chan string, numConsumers),
		buffer:     make([]string, 0),
		buffertype: buffertype,
	}

	switch buffertype {
	case Queue:
		ch.openQueue()
	case Stack:
		ch.openStack()
	}

	return ch
}

// Opens up the buffer for use as a queue by starting a routine that manages the buffer.
func (buffman *BufferManager) openQueue() {
	go func() {
		for {
			if len(buffman.buffer) == 0 {
				// if the queue is empty, cannot feed any elements through the out
				// channel so block on waiting for new elements
				newData := <-buffman.in
				buffman.buffer = append(buffman.buffer, newData)
			} else {
				// if the queue is not empty, block on waiting for new elements or
				// feeding elements to consumers
				select {
				case newData := <-buffman.in:
					buffman.buffer = append(buffman.buffer, newData)
				case buffman.out <- buffman.buffer[0]:
					buffman.buffer = buffman.buffer[1:]
				}
			}
		}
	}()
}

// Opens up the buffer for use as a stack by starting a routine that manages the buffer.
func (buffman *BufferManager) openStack() {
	go func() {
		for {
			if len(buffman.buffer) == 0 {
				// if the stack is empty, cannot feed any elements through the out
				// channel so block on waiting for new elements
				newData := <-buffman.in
				buffman.buffer = append(buffman.buffer, newData)
			} else {
				// if the stack is not empty, block on waiting for new elements or
				// feeding elements to consumers
				select {
				case newData := <-buffman.in:
					buffman.buffer = append(buffman.buffer, newData)
				case buffman.out <- buffman.buffer[len(buffman.buffer)-1]:
					buffman.buffer = buffman.buffer[:len(buffman.buffer)-1]
				}
			}
		}
	}()
}

// Returns the out channel
func (buffman *BufferManager) Out() <-chan string {
	return buffman.out
}
