/* Package logger implements an intuitive global initialized logger
 * 	utilizes the zerolog logger based off of Uber's zap logger
 */
package logger

import (
	"fmt"
	"os"
	"reflect"
	"runtime"
	"strings"
	"time"

	"github.com/rs/zerolog"
)

var Log zerolog.Logger

// Initialize our pretty print logger
func init() {
	zerolog.TimeFieldFormat = zerolog.TimeFormatUnix

	// Create a custom ConsoleWriter object
	output := zerolog.ConsoleWriter{Out: os.Stdout, TimeFormat: time.RFC3339}

	// Custom formatters for our ConsoleWriter
	output.FormatLevel = func(i interface{}) string {
		return strings.ToUpper(fmt.Sprintf("| %-6s|", i))
	}
	output.FormatMessage = func(i interface{}) string {
		return fmt.Sprintf(" >> %s", i)
	}
	output.FormatFieldName = func(i interface{}) string {
		return fmt.Sprintf("[%s]", i)
	}
	output.FormatFieldValue = func(i interface{}) string {
		return strings.ToUpper(fmt.Sprintf("=%s", i))
	}
	output.FormatCaller = func(i interface{}) string {
		return fmt.Sprintf(" >> %s", i)
	}

	// Caller() addsfile and line number to log
	Log = zerolog.New(output).With().Timestamp().Caller().Logger()
	zerolog.SetGlobalLevel(zerolog.InfoLevel)
}

// Retrieve the name of a function
func GetFuncName(v interface{}) string {
	return runtime.FuncForPC(reflect.ValueOf(v).Pointer()).Name()
}
