// Package logger implements an intuitive global initialized logger
//
// Utilizes the zerolog logger based off of Uber's zap logger.
//
// See the provided README.md in go_flexgen/logger/ for more verbose usage.
package logger

import (
	"encoding/json"
	"fmt"
	"os"
	"reflect"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/rs/zerolog"
)

type Config struct {
	Moniker           string   // Make this name the executable in all caps <ex. cops> generated from Initialize()
	ToConsole         bool     `json:"to_console"`          // Do you want to log to screen. Default: false. Overrwritten by dev config.
	ToFile            bool     `json:"to_file"`             // Do you want to log to file. Default: true. Overrwritten by dev config.
	SeverityThreshold Severity `json:"severity_threshold"`  // Print logs above x threshold. See the severity "enum". Default: 1, overwrite with config
	RedundantRateSecs int      `json:"redundant_rate_secs"` // How often the same message will be logged (Seconds)
	ClearRateMins     int      `json:"clear_rate_mins"`     // How often the record of messages will be cleared (Mins)
	FileName          string   // Log to this file generated from Initialize()
}

type Severity int8 // Used with severity enum directly following

// "Enum"
const (
	PANIC Severity = 5
	FATAL Severity = 4
	ERROR Severity = 3
	WARN  Severity = 2
	INFO  Severity = 1
	DEBUG Severity = 0
	TRACE Severity = -1
)

var severityNames = map[Severity]string{
	TRACE: "Trace",
	DEBUG: "Debug",
	INFO:  "Info",
	WARN:  "Warn",
	ERROR: "Error",
	FATAL: "Fatal",
	PANIC: "Panic",
}

// Type of log
type logType int8

const (
	msgLog logType = iota
	errLog
	fullLog
)

type recordEntry struct {
	timestamp time.Time
	count     uint
}

var ConfigFile string // A file optionally passed from the command line that changes default log behavior

var internalZerologLogger zerolog.Logger // Endgame logger. Can be an amalgam of many different sinks.
var severityThreshold Severity           // Needed to make logs only log when severity greater than this var
var redundantRate time.Duration          // Minimum amount of time before logging the same message again
var records map[string]*recordEntry      // Record of all non-exiting messages
var clearRate time.Duration              // Minimum amount of time before clearing the map of messages
var lastRecordsClear = time.Now()        // Last time the records map was cleared
var loggerLock sync.Mutex                // lock that protects the logger's internal state when being accessed by external logic, ensures concurrent use of the package is safe

// Initialize the Logger config with default values. This config will then be used to initialize the actual logger
func InitConfig(processName string) *Config {
	return &Config{
		Moniker:           processName,
		ToConsole:         true,
		ToFile:            true,
		SeverityThreshold: INFO,
		RedundantRateSecs: 10,
		ClearRateMins:     60,
		FileName:          "/var/log/flexgen/" + processName + "/" + processName + ".log",
	}
}

// Needs to be called in main before loggging can be performed.
// The processName should be the service name <ex. "cops">.
// This will be used to create the file location and log file names.
// <"cops" becomes -> /var/log/flexgen/cops/cops.log>
// It will also be used when logging messages to screen/file to indicate the service doing the logging.
func (conf *Config) Init(processName string) (err error) {
	if ConfigFile != "" {
		err = conf.citeConfig(ConfigFile)
	}

	if err != nil {
		return err
	}

	// Create a custom ConsoleWriter object for use if Config.logToFile is true
	output := zerolog.ConsoleWriter{Out: os.Stdout, TimeFormat: time.RFC3339}

	// Custom formatters for ConsoleWriter
	output.FormatTimestamp = func(i interface{}) string {
		return fmt.Sprintf("\n%-6s ", i)
	}
	output.FormatLevel = func(i interface{}) string {
		return strings.ToUpper(fmt.Sprintf("[%s] | %-6s|\n", conf.Moniker, i))
	}
	output.FormatMessage = func(i interface{}) string {
		return fmt.Sprintf("\tMessage: %s\n", i)
	}
	output.FormatFieldName = func(i interface{}) string {
		return fmt.Sprintf("[%s]", i)
	}
	output.FormatFieldValue = func(i interface{}) string {
		return strings.ToUpper(fmt.Sprintf("=%s", i))
	}
	output.FormatCaller = func(i interface{}) string {
		return fmt.Sprintf("\tCaller: %s\n", i)
	}

	// Shorten caller file name caller will be used to provide file name and line.
	zerolog.CallerMarshalFunc = func(file string, line int) string {
		short := file
		for i := len(file) - 1; i > 0; i-- {
			if file[i] == '/' {
				short = file[i+1:]
				break
			}
		}
		file = short
		return file + ":" + strconv.Itoa(line)
	}

	// Create writer based on log destinations
	if conf.ToFile {
		// Make dir if needed
		dir := "/var/log/flexgen/" + processName
		_, err = os.Stat(dir)
		if os.IsNotExist(err) {
			err = os.MkdirAll(dir, 0777)
			if err != nil {
				return fmt.Errorf("failed to make logging folder: %w", err)
			}
		}

		// Open/create the destination file.
		logfile, err := os.OpenFile(conf.FileName, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0666)
		if err != nil {
			return fmt.Errorf("cannot open logfile %s: %w", conf.FileName, err)
		}
		if conf.ToConsole { // to file, to console
			// Multi destination writer
			multi := zerolog.MultiLevelWriter(logfile, output)
			internalZerologLogger = zerolog.New(multi).With().Caller().Logger() // Caller() addsfile and line number to log
		} else { // to file, not to console
			internalZerologLogger = zerolog.New(logfile).With().Caller().Logger()
		}
	} else if conf.ToConsole { // not to file, to console
		internalZerologLogger = zerolog.New(output).With().Caller().Logger()
	} else { // not to file, not to console
		internalZerologLogger = zerolog.Nop().With().Caller().Logger()
	}

	thresh, err := severityToLevel(conf.SeverityThreshold)
	if err != nil {
		return fmt.Errorf("an error occured when setting severity threshold on go_logger for executable %s", processName)
	} else {
		severityThreshold = conf.SeverityThreshold // global int that controls custom logs
		zerolog.SetGlobalLevel(thresh)
	}

	redundantRate = time.Second * time.Duration(conf.RedundantRateSecs)
	if redundantRate < 0 {
		redundantRate = time.Duration(0)
	}

	clearRate = time.Minute * time.Duration(conf.ClearRateMins)
	if clearRate < 0 {
		clearRate = time.Duration(0)
	}

	Startup()
	return nil
}

// Overwrites the config struct with user provided json file values
func (conf *Config) citeConfig(filePath string) error {
	file, err := os.Open(filePath)
	if err != nil {
		return err
	}
	defer file.Close()
	decoder := json.NewDecoder(file)
	err = decoder.Decode(conf)
	if err != nil {
		return err
	}
	return nil
}

// Convert int8/Severity type to zerolog equivalent
func severityToLevel(sev Severity) (zerolog.Level, error) {
	switch sev {
	case -1:
		return zerolog.TraceLevel, nil
	case 0:
		return zerolog.DebugLevel, nil
	case 1:
		return zerolog.InfoLevel, nil
	case 2:
		return zerolog.WarnLevel, nil
	case 3:
		return zerolog.ErrorLevel, nil
	case 4:
		return zerolog.FatalLevel, nil
	case 5:
		return zerolog.PanicLevel, nil
	default:
		return zerolog.DebugLevel, fmt.Errorf("a value outside possible severity constraints was passed. -1<=x<=5")
	}
}

// Startup message that should always be the first message logged. This helps indicate a new run after the service crashes.
func Startup() {
	zerolog.CallerSkipFrameCount = 3
	records = make(map[string]*recordEntry)
	internalZerologLogger.Log().Timestamp().Str("level", "STARTUP ").Msg("File Logger startup!")
	zerolog.CallerSkipFrameCount = 2
}

// Update records with the message received, returning true if the message should be logged as well
// as an optional string, indicating the number of duplicate records, that will be appended to the log.
func updateRecords(msgKey string) (shouldLog bool, redundancyCountAlert string) {
	// if msg is not in record, add it and allow it to be logged
	lastRecord, ok := records[msgKey]
	if !ok {
		records[msgKey] = &recordEntry{time.Now(), 0}
		return true, ""
	}

	// block redundant messages during redundancy period
	if time.Since(lastRecord.timestamp) <= redundantRate {
		lastRecord.count++
		return false, ""
	}

	// refresh record and return redundancy count alert if non-zero
	lastRecord.timestamp = time.Now()
	if lastRecord.count == 0 {
		return true, ""
	}
	redundancyCountAlert = fmt.Sprintf(" seen %d more times in redundant period", lastRecord.count)
	lastRecord.count = 0
	return true, redundancyCountAlert
}

// Clears the records.
func clearRecords() {
	records = make(map[string]*recordEntry)
	lastRecordsClear = time.Now()
}

// Log message based on the arguments provided, and update records as appropriate
func logMsg(ltype logType, level Severity, msg string, err error) {
	loggerLock.Lock()
	defer loggerLock.Unlock()

	// Only log if severity meets the required threshold
	if level < severityThreshold {
		return
	}

	// We only care about redundancy checks if our severity threshold is greater than DEBUG.
	// If we are at DEBUG or below, then we want to see every log.
	shouldUpdateRecords := (severityThreshold > DEBUG)

	shouldLog := true
	redundantMsg := ""

	zerolog.CallerSkipFrameCount = 4
	switch ltype {
	case msgLog:
		if shouldUpdateRecords {
			shouldLog, redundantMsg = updateRecords(msg)
		}
		if !shouldLog {
			return
		}
		internalZerologLogger.Log().Timestamp().Str("level", severityNames[level]).Msg(msg + redundantMsg)
	case errLog:
		if shouldUpdateRecords {
			shouldLog, redundantMsg = updateRecords(err.Error())
		}
		if !shouldLog {
			return
		}
		internalZerologLogger.Log().Timestamp().Str("level", severityNames[level]).Err(err).Msg(msg + redundantMsg)
	case fullLog:
		if shouldUpdateRecords {
			shouldLog, redundantMsg = updateRecords(msg + err.Error())
		}
		if !shouldLog {
			return
		}
		internalZerologLogger.Log().Timestamp().Str("level", severityNames[level]).Err(err).Msg(msg + redundantMsg)
	}
	zerolog.CallerSkipFrameCount = 2

	// Reset records if clear time has been exceeded
	if time.Since(lastRecordsClear) >= clearRate {
		clearRecords()
	}
}

// Debug log Level

// Printf style debug
func Debugf(format string, args ...interface{}) {
	logMsg(msgLog, DEBUG, fmt.Sprintf(format, args...), nil)
}

// Provide only a string
func MsgDebug(msg string) {
	logMsg(msgLog, DEBUG, msg, nil)
}

// Provide only an error
func ErrDebug(err error) {
	logMsg(errLog, DEBUG, "", err)
}

// Provide both a string and an error
func FullDebug(msg string, err error) {
	logMsg(fullLog, DEBUG, msg, err)
}

// Info log level

// Printf style Info
func Infof(format string, args ...interface{}) {
	logMsg(msgLog, INFO, fmt.Sprintf(format, args...), nil)
}

// Provide only a string
func MsgInfo(msg string) {
	logMsg(msgLog, INFO, msg, nil)
}

// Provide only an error
func ErrInfo(err error) {
	logMsg(errLog, INFO, "", err)
}

// Provide both a string and an error
func FullInfo(msg string, err error) {
	logMsg(fullLog, INFO, msg, err)
}

// Warn log level

// Printf style Warn
func Warnf(format string, args ...interface{}) {
	logMsg(msgLog, WARN, fmt.Sprintf(format, args...), nil)
}

// Provide only a string
func MsgWarn(msg string) {
	logMsg(msgLog, WARN, msg, nil)
}

// Provide only an error
func ErrWarn(err error) {
	logMsg(errLog, WARN, "", err)
}

// Provide both a string and an error
func FullWarn(msg string, err error) {
	logMsg(fullLog, WARN, msg, err)
}

// Error log level.

// Printf style Error
func Errorf(format string, args ...interface{}) {
	logMsg(msgLog, ERROR, fmt.Sprintf(format, args...), nil)
}

// Provide only a string
func MsgError(msg string) {
	logMsg(msgLog, ERROR, msg, nil)
}

// Provide only an error
func ErrError(err error) {
	logMsg(errLog, ERROR, "", err)
}

// Provide both a string and an error
func FullError(msg string, err error) {
	logMsg(fullLog, ERROR, msg, err)
}

// Fatal log level.

// Printf style Fatal, calls os.Exit(-1) after logging. This will cause the program to stop immediatly.
func Fatalf(format string, args ...interface{}) {
	logMsg(msgLog, FATAL, fmt.Sprintf(format, args...), nil)
	os.Exit(-1)
}

// Provide only a string, calls os.Exit(-1) after logging. This will cause the program to stop immediatly.
func MsgFatal(msg string) {
	logMsg(msgLog, FATAL, msg, nil)
	os.Exit(-1)
}

// Provide only an error, calls os.Exit(-1) after logging. This will cause the program to stop immediatly.
func ErrFatal(err error) {
	logMsg(errLog, FATAL, "", err)
	os.Exit(-1)
}

// Provide both a string and an error, calls os.Exit(-1) after logging. This will cause the program to stop immediatly.
func FullFatal(msg string, err error) {
	logMsg(fullLog, FATAL, msg, err)
	os.Exit(-1)
}

// Panic log level.

// Printf style Panic, calls panic() after logging. This will cause the program to stop and unwind the stack, defers and other cleanup can still be executed.
func Panicf(format string, args ...interface{}) {
	msg := fmt.Sprintf(format, args...)
	logMsg(msgLog, PANIC, msg, nil)
	panic("A panic occured after logging the following: " + msg)
}

// Provide only a string, calls panic() after logging. This will cause the program to stop and unwind the stack, defers and other cleanup can still be executed.
func MsgPanic(msg string) {
	logMsg(msgLog, PANIC, msg, nil)
	panic("A panic occured after logging the following message: " + msg)
}

// Provide only an error, calls panic() after logging. This will cause the program to stop and unwind the stack, defers and other cleanup can still be executed.
func ErrPanic(err error) {
	logMsg(errLog, PANIC, "", err)
	panic("A panic occured after logging the following error: " + err.Error())
}

// Provide both a string and an error, calls panic() after logging. This will cause the program to stop and unwind the stack, defers and other cleanup can still be executed.
func FullPanic(msg string, err error) {
	logMsg(fullLog, PANIC, msg, err)
	panic("A panic occured after logging the following message: " + msg + ", and the following error: " + err.Error())
}

// Trace log level.

// Printf style Trace
func Tracef(format string, args ...interface{}) {
	logMsg(msgLog, TRACE, fmt.Sprintf(format, args...), nil)
}

// Provide only a string
func MsgTrace(msg string) {
	logMsg(msgLog, TRACE, msg, nil)
}

// Provide only an error
func ErrTrace(err error) {
	logMsg(errLog, TRACE, "", err)
}

// Provide both a string and an error
func FullTrace(msg string, err error) {
	logMsg(fullLog, TRACE, msg, err)
}

// Retrieve the name of a function. Does not read from, or write to, the logger state.
func GetFuncName(v interface{}) string {
	return runtime.FuncForPC(reflect.ValueOf(v).Pointer()).Name()
}

// callback function for loglevel api
func SetLogLevel(args []interface{}) (string, error) {
	loggerLock.Lock()
	defer loggerLock.Unlock()

	validRange := "<panic|fatal|error|warn|info|debug|trace|(numeric 5 to -1)>"
	if len(args) != 1 {
		return "", fmt.Errorf("set-log-level takes 1 argument: %s but %d args provided", validRange, len(args))
	}
	var severity Severity
	logStr := strings.ToLower(args[0].(string))
	switch logStr {
	case "panic":
		severity = PANIC
	case "fatal":
		severity = FATAL
	case "error":
		severity = ERROR
	case "warn":
		severity = WARN
	case "info":
		severity = INFO
	case "debug":
		severity = DEBUG
	case "trace":
		severity = TRACE
	default:
		// Handle numeric case
		intval, err := strconv.Atoi(logStr)
		if err != nil {
			// parse error
			return "", fmt.Errorf("failed to convert argument %s to loglevel %s: %w", logStr, validRange, err)
		}
		if severity < -1 || severity > 5 {
			return "", fmt.Errorf("loglevel %d is out of range: %s", severity, validRange)
		}
		severity = Severity(intval)
	}
	severityThreshold = severity
	zerolog.SetGlobalLevel(zerolog.Level(severity))
	return "loglevel set to " + logStr + "\n", nil
}

// callback function for Redundant Rate api
func SetRedundantRate(args []interface{}) (string, error) {
	loggerLock.Lock()
	defer loggerLock.Unlock()

	validValue := "<Rate in seconds (int)>"
	if len(args) != 1 {
		return "", fmt.Errorf("set-redundant-rate takes 1 argument: %s but %d args provided", validValue, len(args))
	}

	stringArg, ok := args[0].(string)
	newRate, err := strconv.Atoi(stringArg)
	if !ok || err != nil {
		return "", fmt.Errorf("failed to convert argument %s to redundantRate %s: %w", stringArg, validValue, err)
	}
	if newRate < 0 {
		newRate = 0
	}
	redundantRate = time.Duration(newRate)
	return fmt.Sprintf("RedundantRateS set to %d\n", newRate), nil
}

// callback function for Clear Rate api
func SetClearRate(args []interface{}) (string, error) {
	loggerLock.Lock()
	defer loggerLock.Unlock()

	validValue := "<Rate in mins (int)>"
	if len(args) != 1 {
		return "", fmt.Errorf("set-clear-rate takes 1 argument: %s but %d args provided", validValue, len(args))
	}

	stringArg, ok := args[0].(string)
	newRate, err := strconv.Atoi(stringArg)
	if !ok || err != nil {
		return "", fmt.Errorf("failed to convert argument %s to redundantRate %s: %w", stringArg, validValue, err)
	}
	if newRate < 0 {
		newRate = 0
	}
	clearRate = time.Duration(newRate)
	return fmt.Sprintf("ClearRateM set to %d\n", newRate), nil
}
