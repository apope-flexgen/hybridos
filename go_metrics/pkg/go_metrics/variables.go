package go_metrics

import (
	"bytes"
	"encoding/json"
	"fims"
	"sync"
	"time"

	simdjson "github.com/minio/simdjson-go"
)

// metrics globals
var MetricsConfig MetricsFile     // the metrics config file
var SubscribeUris []string        // the uris to subscribe to
var InputScope map[string][]Union // this holds a map[string][]Input for when we want to go and evaluate things...
var FilterScope map[string][]string
var FiltersList []string               // this maintains the order in which we evaluate filters
var OutputScope map[string][]Union     // this holds a map[string][]Input for when we want to go and evaluate things...
var UriElements map[string]interface{} // if we decompose a uri, use this to get the elements of that uri that we're interested in -- used for inputs
var PublishUris map[string][]string    // the uri's to publish to and the vars to publish to each uri
var PubUriFlags map[string][]string    // the uri's to publish to and the flags contained within that publish uri (either local or global)
var ConfigErrorsFile string            // the file location to save the error report after parsing the config document
var MdoFile string                     // the file location to save the "metrics data object" (MDO)
var Mdo MDO                            // the struct that holds all of the data that we need to save to the MDO
var ProcessName string                 // the process name to publish as and subscribe to
var ConfigSource string                // where the config file for the program is located

// mutexes
var inputScopeMutex sync.RWMutex
var outputScopeMutex sync.RWMutex
var metricsMutex []sync.RWMutex
var expressionNeedsEvalMutex sync.RWMutex
var filtersMutex sync.RWMutex
var echoMutex sync.RWMutex
var filterNeedsEvalMutex sync.RWMutex
var pubDataChangedMutex sync.RWMutex
var echoMsgBodyMutex sync.RWMutex
var directMsgMutex sync.RWMutex
var elementValueMutex sync.Mutex

// local variables used throughout the package
// since we want to minimize garbage collection
var allOutputFlags = []string{"naked", "clothed", `group\d+`, "interval_set", "enum", "bitfield", "sparse", "no_heartbeat", "direct_set", "post", "lonely"}
var uriIsSparse map[string]bool
var uriIsIntervalSet map[string]bool
var uriIsLonely map[string]bool
var uriIsDirect map[string]map[string]bool          // map of uris that send direct messages (set or post)
var uriToDirectMsgActive map[string]map[string]bool // map of uris that have direct messages to whether or not we need to send a pub out now. The outer map indicates the method (set or post) and the inner map indicates the uri and number of messages
var uriHeartbeat map[string]bool
var uriToInputNameMap map[string][]string             // a map of uri's to input variable names
var uriToEchoObjectInputMap map[string]map[int]int    // a map of uri's to listen for and corresponding echo objects that use them
var uriToOutputNameMap map[string][]string            // a map of uri's to output variable names. A uri can have multiple outputs associated with it
var allPossibleAttributes map[string][]string         // get a map of all possible attribute names (e.g. enabled, scale, etc) to their attribute name in the metricsConfig attributes map
var staticFilterExpressions map[string]Filter         // this holds a map of "filtered" variable names to the expressions required to evaluate them; these can be evaluated prior to runtime
var dynamicFilterExpressions map[string]Filter        // this holds a map of "filtered" variable names to the expressions required to evaluate them; these must be evaluated at runtime
var inputToMetricsExpression map[string][]int         // when variables change their values (and only when), we will re-evaluate the corresponding expressions
var inputToFilterExpression map[string][]string       // when variables change their values (and only when), we will re-evaluate the corresponding filters (which may result in new expressions to evaluate)
var outputToMetricsObject map[string][]*MetricsObject // the user can send a signal to the output that will trigger a re-evaluation of the metric
var expressionNeedsEval map[int]bool                  // map of expressions that need re-evaluation (true means needs re-eval)
var filterNeedsEval map[string]bool                   // map of dynamic expressions that need re-evaluation (true means needs re-eval)
var pubDataChanged map[string]bool                    // if data in a publish uri changes, we need to send out a new pub!!
var outputToUriGroup map[string]string                // since outputs can get published to the same uri but at different times, we need this to keep track of the groups
var outputVarChanged map[string]bool                  // if an output variable changed, we want to keep track of it for sparse publishes
var noGetResponse map[string]bool                     // don't respond to get requests for this uri
var echoOutputToInputNum map[string]int               // full echo register uri to input num
var echoPublishUristoEchoNum map[string]int           // the pub uri to the echo object number

// local variables used for publishing messages
var directMsgBody map[string]interface{}
var pubMsgBody map[string]interface{}
var echoMsgBody map[string]interface{}
var tickers [](*time.Ticker)    // tickers that keep track of publishing rates
var pubTickers map[string]int   // a map of publish uris to ticker index
var tickerPubs map[int][]string // a map of ticker index to publish uris

// fims globals
var f fims.Fims
var fimsMap chan fims.FimsMsgRaw

// global globals
var configPathAndFile string
var debug bool
var debug_inputs []string
var debug_filters []string
var debug_outputs []string
var echoInput EchoInput
var mdooutput Output
var tempValue interface{}
var mdoBuf *bytes.Buffer
var mdoEncoder *json.Encoder
var containedInValChanged map[string]bool
var inputYieldsDirectMsg map[string]bool

// runtime parsing variables - for process_fims
var pj *simdjson.ParsedJson
var iter simdjson.Iter
var elementValue interface{}
var msgBodyIn interface{}
var msgBodyInMutex sync.RWMutex
var iterMap map[string]*simdjson.Iter
var objMap map[string]*simdjson.Object
var elemMap map[string]*simdjson.Iter

// timing variables
var processFimsTiming Timing
var evalExpressionsTiming Timing
var t0 time.Time
