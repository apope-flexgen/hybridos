package go_metrics

import (
	"math"
	"sync"
	"time"
)

type Timing struct {
	mutex       sync.RWMutex
	startTime   time.Time
	elapsedTime int64
	count       int64
	total       int64
	min         int64
	max         int64
	average     int64
}

func (t *Timing) init() {
	t.min = math.MaxInt64
	t.max = 0
	t.count = 0
	t.total = 0
	t.average = 0
}

func (t *Timing) start() {
	t.mutex.Lock()
	t.startTime = time.Now()
	t.mutex.Unlock()
}

func (t *Timing) stop() {
	t.mutex.Lock()
	t.elapsedTime = time.Since(t.startTime).Nanoseconds()
	t.count += 1
	t.total += t.elapsedTime
	if t.elapsedTime < t.min {
		t.min = t.elapsedTime
	}
	if t.elapsedTime > t.max {
		t.max = t.elapsedTime
	}
	t.mutex.Unlock()
}

func (t *Timing) calculateAverage() {
	t.mutex.Lock()
	if t.count > 0 {
		t.average = t.total / t.count
	}
	t.mutex.Unlock()
}
