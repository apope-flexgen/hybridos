package main

import (
	"fmt"
	"time"

	"github.com/flexgen-power/hybridos/go_flexgen/buffman"
)

var messages = []string{
	"The world itself's",
	"just one big hoax.",
	"Spamming each other with our",
	"running commentary of bullshit,",
	"masquerading as insight, our social media",
	"faking as intimacy.",
	"Or is it that we voted for this?",
	"Not with our rigged elections,",
	"but with our things, our property, our money.",
	"I'm not saying anything new.",
	"We all know why we do this,",
	"not because Hunger Games",
	"books make us happy,",
	"but because we wanna be sedated.",
	"Because it's painful not to pretend,",
	"because we're cowards.",
	"- Elliot Alderson",
	"Mr. Robot",
}

func main() {
	inQ := make(chan string)
	queue := buffman.New(buffman.Queue, 1, inQ)
	go func() {
		for _, m := range messages {
			inQ <- m
		}
		fmt.Println("\tDone adding jobs to buffer manager.")
	}()

	numJobsCompleted := 0
	for job := range queue.Out() {
		fmt.Println(job)
		time.Sleep(time.Second / 3)
		numJobsCompleted++
		if numJobsCompleted == len(messages) {
			break
		}
	}

	inS := make(chan string)
	stack := buffman.New(buffman.Stack, 1, inS)
	go func() {
		for i := range messages {
			inS <- fmt.Sprint(i)
		}
		fmt.Println("\tDone adding jobs to buffer manager.")
	}()

	numJobsCompleted = 0
	for job := range stack.Out() {
		fmt.Println(job)
		time.Sleep(time.Second / 3)
		numJobsCompleted++
		if numJobsCompleted == len(messages) {
			break
		}
	}
}
