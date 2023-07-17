package main

import (
	"fmt"
	"sort"
	"strings"
	"text/tabwriter"
)

type conflictTable struct {
	table  [][]bool
	legend []([]string)
}

// Create a table indicating type conflicts between formats
func createUriFormatConflictTable(formatStringToUris map[string]map[string]struct{}, formatStringToFormat map[string]uriMsgFormat) conflictTable {
	// get the uris sorted alphanumerically
	type uriGroup struct {
		format uriMsgFormat
		uris   []string
	}

	uriGroups := make([]uriGroup, 0)
	for formatString, uriSet := range formatStringToUris {
		uris := make([]string, 0)
		for uri := range uriSet {
			uris = append(uris, uri)
		}
		sort.Strings(uris)
		uriGroups = append(uriGroups, uriGroup{format: formatStringToFormat[formatString], uris: uris})
	}
	sort.Slice(uriGroups, func(i, j int) bool {
		return uriGroups[i].uris[0] < uriGroups[j].uris[0]
	})

	// construct the legend
	urisLegend := make([]([]string), len(uriGroups))
	for i, uriGroup := range uriGroups {
		urisLegend[i] = uriGroup.uris
	}

	// construct the table by comparing every pair of formats
	table := make([][]bool, len(uriGroups))
	for i, firstUriGroup := range uriGroups {
		table[i] = make([]bool, len(uriGroups))
		for j, secondUriGroup := range uriGroups {
			table[i][j] = firstUriGroup.format.hasConflictWith(secondUriGroup.format)
		}
	}

	return conflictTable{table: table, legend: urisLegend}
}

// Create a string representation of a conflict table
func (ctable conflictTable) string() string {
	var repBuilder strings.Builder

	// write the table
	tableWriter := tabwriter.NewWriter(&repBuilder, 1, 4, 1, ' ', 0)
	// write column labels
	fmt.Fprintf(tableWriter, " \t")
	for col := range ctable.table[0] {
		fmt.Fprintf(tableWriter, "%d\t", col)
	}
	fmt.Fprintln(tableWriter)
	for row := range ctable.table {
		fmt.Fprintf(tableWriter, "%d\t", row)
		for _, hasConflict := range ctable.table[row] {
			if hasConflict {
				fmt.Fprintf(tableWriter, "X\t")
			} else {
				fmt.Fprintf(tableWriter, "-\t")
			}
		}
		fmt.Fprintln(tableWriter)
	}
	tableWriter.Flush()

	// write the legend
	for i, uris := range ctable.legend {
		repBuilder.WriteString(fmt.Sprintf("| %d: %v\n", i, uris))
	}

	return repBuilder.String()
}
