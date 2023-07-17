package main

import (
	"fmt"
	"sort"
	"strings"
)

type uriFormatForest []*uriFormatNode

// A node in a forest of uri formats organized vertically by format subsets
type uriFormatNode struct {
	format   uriMsgFormat     // the format of the uris
	uris     []string         // the uris associated with this format
	children []*uriFormatNode // the format nodes with formats that are subsets of this format node's format
}

// Creates a forest of the uri formats organized such that each parent is a subset of its child
func createUriFormatSubsetForest(formatStringToUris map[string]map[string]struct{}, formatStringToFormat map[string]uriMsgFormat) uriFormatForest {
	roots := make([]*uriFormatNode, 0)
	// for each format and its uris, create a node and add it to the forest
	for formatString, urisSet := range formatStringToUris {
		node := &uriFormatNode{
			format: formatStringToFormat[formatString],
			uris:   make([]string, 0),
		}
		for uri := range urisSet {
			node.uris = append(node.uris, uri)
		}
		roots = addNodeToUriFormatSubsetForest(node, roots)
	}
	// sort for readability and more consistent results
	roots = sortUriFormatForestAlphanumeric(roots)
	return roots
}

// Sorts the forest such that uris appear in alphanumeric order
func sortUriFormatForestAlphanumeric(forest uriFormatForest) (sortedForest uriFormatForest) {
	sortedForest = forest
	// sort the uris list for each root node
	for _, root := range sortedForest {
		sort.Strings(root.uris)
	}
	// sort the root nodes by alphanumeric order of first uri in uris list
	sort.Slice(sortedForest, func(i, j int) bool { return sortedForest[i].uris[0] < sortedForest[j].uris[0] })
	// sort each of the root nodes' children
	for _, root := range sortedForest {
		root.children = sortUriFormatForestAlphanumeric(root.children)
	}
	return sortedForest
}

// Determine where the node should go and add it to the forest
func addNodeToUriFormatSubsetForest(node *uriFormatNode, forest uriFormatForest) (newForest uriFormatForest) {
	if forest == nil {
		newForest = []*uriFormatNode{node}
		return newForest
	}
	// compare the node against each root of the forest
	for _, root := range forest {
		if root.format.isSubsetOf(node.format) {
			// node should be a child
			newRootChildren := addNodeToUriFormatSubsetForest(node, root.children)
			root.children = newRootChildren
			return forest
		} else if node.format.isSubsetOf(root.format) {
			// node should be a parent
			// handle the possibility of the node being a parent of multiple roots by finding all of them
			// and adding them as children
			node.children = []*uriFormatNode{}
			indexesOfRemainingRoots := make(map[int]struct{})
			for i, potentialChild := range forest {
				if node.format.isSubsetOf(potentialChild.format) {
					node.children = append(node.children, potentialChild)
				} else {
					indexesOfRemainingRoots[i] = struct{}{}
				}
			}
			newForest = []*uriFormatNode{}
			// remove all the roots that are no longer roots
			for i, oldRoot := range forest {
				if _, shouldRemain := indexesOfRemainingRoots[i]; shouldRemain {
					newForest = append(newForest, oldRoot)
				}
			}
			// add node as a root
			newForest = append(newForest, node)
			return newForest
		}
		// else node is not comparable to this root
	}
	// if the node doesn't belong in any tree, make it a new root
	newForest = append(forest, node)
	return newForest
}

// Creates a string representation of a uri format forest with a given tab count
func stringRepresentingUriFormatForest(forest uriFormatForest, tabCount int) string {
	var stringRepBuilder strings.Builder
	writeWithTab := func(s string) {
		for i := 0; i < tabCount; i++ {
			stringRepBuilder.WriteString("--")
		}
		stringRepBuilder.WriteString(s)
	}

	// add a string for each root
	for _, root := range forest {
		// write the uris on a line
		writeWithTab("| ")
		for _, uri := range root.uris {
			stringRepBuilder.WriteString(fmt.Sprintf("%s, ", uri))
		}
		stringRepBuilder.WriteString("\n")
		// if there are children, add a string for them with one more tab
		if root.children != nil {
			stringRepBuilder.WriteString(stringRepresentingUriFormatForest(root.children, tabCount+1))
		}
	}

	return stringRepBuilder.String()
}

func (forest uriFormatForest) string() string {
	return stringRepresentingUriFormatForest(forest, 0)
}
