// A small demo program to show the various operations one can do to manipulate an ID cache.
package main

import (
	"bufio"
	"errors"
	"fmt"
	"log"
	"os"

	"github.com/flexgen-power/scheduler/internal/idcache"
)

func main() {
	cache := idcache.New()
	for {
		printCommandPrompt()
		userInput, err := getUserInput()
		if err != nil {
			log.Fatalf("Error getting user input: %v.", err)
		}

		switch userInput[0] {
		case 'p':
			if len(cache) == 0 {
				fmt.Println("Cache is empty.")
			} else {
				for id := range cache {
					fmt.Println(id)
				}
			}
		case 'g':
			newId, err := cache.GenerateId()
			if err != nil {
				fmt.Printf("Error generating new ID: %v.\n", err)
			} else {
				fmt.Printf("Added new ID %v to cache.\n", newId)
			}
		case 'c':
			if len(userInput) < 3 {
				fmt.Printf("Check command requires a 'c' followed by a space and then an ID for which to check.\n")
				continue
			}
			targetIdString := userInput[2:]
			targetId, err := idcache.ParseIdFromString(targetIdString)
			if err != nil {
				fmt.Printf("Could not parse ID from string %s: %v.", targetIdString, err)
				continue
			}
			if cache.CheckId(targetId) {
				fmt.Printf("ID %v is in the cache.\n", targetId)
			} else {
				fmt.Printf("ID %v is NOT in the cache.\n", targetId)
			}
		case 'd':
			if len(userInput) < 3 {
				fmt.Printf("Delete command requires a 'd' followed by a space and then an ID to delete.\n")
				continue
			}
			targetIdString := userInput[2:]
			targetId, err := idcache.ParseIdFromString(targetIdString)
			if err != nil {
				fmt.Printf("Could not parse ID from string %s: %v.\n", targetIdString, err)
				continue
			}
			if !cache.CheckId(targetId) {
				fmt.Printf("Cache does not have ID %v.\n", targetId)
				continue
			}
			cache.DeleteId(targetId)
			fmt.Printf("Deleted ID %v from cache.\n", targetId)
		case 'w':
			cache = idcache.New()
			fmt.Printf("Wiped the cache.\n")
		case 'q':
			return
		default:
			fmt.Printf("%s is not a valid command.\n", userInput)
		}
		fmt.Printf("\n")
	}
}

func printCommandPrompt() {
	fmt.Printf("Please select a command.\n")
	fmt.Printf("- Type 'p' to print the current cache.\n")
	fmt.Printf("- Type 'g' to generate a new ID.\n")
	fmt.Printf("- Type 'c <ID>' to check if the ID specified by '<ID>' is in the cache.\n")
	fmt.Printf("- Type 'd <ID>' to delete the ID specified by '<ID>'.\n")
	fmt.Printf("- Type 'w' to wipe the cache.\n")
	fmt.Printf("- Type 'q' to quit.\n")
	fmt.Printf("\n")
}

func getUserInput() (string, error) {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		userInput := scanner.Text()
		if len(userInput) > 0 {
			return userInput, nil
		}
	}
	if err := scanner.Err(); err != nil {
		return "", fmt.Errorf("failed to read standard input: %w", err)
	}
	return "", errors.New("unknown error")
}
