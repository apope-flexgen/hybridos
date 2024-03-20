#include "logger/gcom_dnp3_logger.h"
#include "logger/event_logger.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/pattern_formatter.h"
#include <fstream>
#include <sstream>
#include <cjson/cJSON.h>
#include <chrono>
#include <tuple>

std::chrono::steady_clock::time_point base_time;
std::mutex logger_mutex;

class elapsed_time_formatter : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        fmt::format_to(std::back_inserter(dest), "{:<{}}", std::chrono::duration<double>(std::chrono::steady_clock::now() - base_time), padinfo_.width_);
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<elapsed_time_formatter>();
    }
};

namespace Logging
{
    // Initialize default config values
    bool init = false;
    bool to_console = true;
    bool to_file = true;
    std::map<spdlog::level::level_enum, std::string> severity_names =
        {
            std::make_pair(spdlog::level::trace, "Trace"),
            std::make_pair(spdlog::level::debug, "Debug"),
            std::make_pair(spdlog::level::info, "Info"),
            std::make_pair(spdlog::level::warn, "Warning"),
            std::make_pair(spdlog::level::err, "Error"),
            std::make_pair(spdlog::level::critical, "Critical"),
    };
    std::string log_dir;
    spdlog::level::level_enum severity_threshold = spdlog::level::info;
    std::chrono::seconds redundant_rate = std::chrono::seconds(10);
    std::chrono::minutes clear_rate = std::chrono::minutes(1);
    std::chrono::_V2::steady_clock::time_point last_records_clear = std::chrono::steady_clock::now();
    // Declare other static objects that will be configured in Init()
    EventLogger logger{"dnp3", 64};
    std::shared_ptr<spdlog::logger> console;
    std::unordered_map<std::string, RecordEntry> records;

    /**
     * Parse command line arguments for config file path
     * @return The config file path found, or an empty string
     */
    std::string parse_config_path(int argc, char **argv)
    {
        std::string config_file_path;
        // Check command line arguments for logCfg flag and filepath
        for (int i = 1; i < argc; ++i)
        {
            std::string current_arg = std::string(argv[i]);
            if (current_arg.find("-logCfg") < std::string::npos)
            {
                // If a condensed config argument has been given: "-logCfg=<filepath>"
                size_t file_start = current_arg.find("=");
                if (file_start < current_arg.length())
                {
                    fprintf(stderr, "split case\n");
                    return config_file_path = current_arg.substr(file_start + 1, current_arg.length());
                }
                // If a flag / filepath pair has been given
                else if (i < argc - 1)
                {
                    fprintf(stderr, "pair case\n");
                    return config_file_path = std::string(argv[i + 1]);
                }
            }
        }
        return config_file_path;
    }

    // init function creates both a console logger and a file logger
    void Init(std::string module, int argc, char **argv)
    {
        init = true;
        base_time = std::chrono::steady_clock::now();
        // make file path based of string passed to init
        log_dir = "/var/log/flexgen/";
        log_dir.append(module);

        // try reading custom config
        std::string config_file_path = parse_config_path(argc, argv);
        std::vector<std::string> config_errs = read_config(config_file_path);

        // This is the file logger
        if (to_file)
        {
            logger = EventLogger(module.c_str(), 64);
            logger.set_level(severity_threshold);                                                                   // log everything
            logger.flush_on(severity_threshold);                                                                    // flush after errors
            logger.setPattern("{\"time\": \"%T.%e\", \"date\": \"%Y-%m-%d\", \"PID\": %P, \"level\": \"%l\", %v}"); // This pattern is NEEDED site_controller is pushing json.
        }

        // This logs to console
        if (to_console)
        {
            console = spdlog::stdout_color_mt(module.c_str());
            console->set_level(severity_threshold); // log everything
            console->flush_on(severity_threshold);  // flush after every log
            auto formatter = std::make_unique<spdlog::pattern_formatter>();
            formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%Y-%m-%d] [%T.%e] [%-8*] [PID %P] [%n] [%^%-8l%$] %v");
            console->set_formatter(std::move(formatter));
        }

        // Report any issues setting up config
        FPS_INFO_LOG("Setting up logger with config from: %s", config_file_path.empty() ? "NULL" : config_file_path.c_str());
        for (auto config_err : config_errs)
            FPS_WARNING_LOG(config_err);
    }

    /**
     * Read config from file
     * @param file_path the file path provided via command line argument
     * @return a string representing any error that occurred, as we are not yet able to log the error
     */
    std::vector<std::string> read_config(std::string file_path)
    {
        // If no file path provided, return successfully
        if (file_path.empty())
            return {"no logger configuration provided, default values used"};

        // Read config file
        std::ifstream log_config_file(file_path);
        if (!log_config_file)
            return {"failed to open config file, default values used"};

        std::stringstream log_config_buffer;
        log_config_buffer << log_config_file.rdbuf();
        if (!log_config_buffer)
            return {"failed to read from config file, default values used"};

        cJSON *log_config = cJSON_Parse(log_config_buffer.str().c_str());
        if (!log_config)
            return {"failed to parse config file, default values used"};

        // Parse each optional config item
        cJSON *item = NULL;
        std::vector<std::string> config_issues;
        if (!(item = cJSON_GetObjectItem(log_config, "to_console")))
            config_issues.push_back("missing field: to_console, defaulted to true");
        else
            to_console = (item->type == cJSON_True);

        if (!(item = cJSON_GetObjectItem(log_config, "to_file")))
            config_issues.push_back("missing field: to_file, defaulted to true");
        else
            to_file = (item->type == cJSON_True);

        if (!(item = cJSON_GetObjectItem(log_config, "severity_threshold")))
            config_issues.push_back("missing field: severity_threshold, defaulted to 1 (Info)");
        else if (item->valueint < MIN_LOG_LEVEL || item->valueint > MAX_LOG_LEVEL)
            config_issues.push_back("invalid field: severity_threshold must be between -1 (Trace) and 5 (Critical), defaulted to 1 (Info)");
        else
            severity_threshold = severity_to_level(item->valueint);

        if (!(item = cJSON_GetObjectItem(log_config, "redundant_rate_secs")))
            config_issues.push_back("missing field: redundant_rate_secs, defaulted to 10 seconds");
        else if (item->valueint < 0)
            config_issues.push_back("invalid field: redundant_rate_secs must be >= 0, defaulted to 10 seconds");
        else
            redundant_rate = std::chrono::seconds(item->valueint);

        if (!(item = cJSON_GetObjectItem(log_config, "clear_rate_mins")))
            config_issues.push_back("missing field: clear_rate_mins, defaulted to 1 minute");
        else if (item->valueint < 0)
            config_issues.push_back("invalid field: clear_rate_mins must be >= 0, defaulted to 1 minute");
        else
            clear_rate = std::chrono::minutes(item->valueint);

        cJSON_Delete(log_config);
        return config_issues;
    }

    /**
     * Generic log function for all log levels and types except TEST, called by the respective FPS_<TYPE>_LOG functions.
     * Messages will only be logged if their severity is at or above the configured severity_threshold.
     * A record of the message and its timestamp will also be stored, and the message will not be logged if it has
     * been seen previously within the redundant_rate period.
     * @param type The destination type of the log, console or file
     * @param severity The severity level of the log, from trace up to critical. Only Info and Error are typically used
     * @param pre Formatted prestring logged before the message
     * @param msg The formatted message
     * @param post Formatted poststring logged after the message
     */
    void log_msg(spdlog::level::level_enum severity, std::string pre, std::string msg_stripped, std::string original_msg, std::string post)
    {
        // Only log if severity meets the required threshold
        if (severity < severity_threshold || !init)
            return;

        // if(!init){
        //     printf("%s\n",original_msg.c_str());
        //     return;
        // }

        // Only check for redundancies if above debug, else we want to see every log
        bool should_update_records = (severity > spdlog::level::debug);
        bool should_log = true;
        std::string redundant_msg;

        if (should_update_records)
            should_log = update_records(msg_stripped, redundant_msg);

        if (to_file)
        {
            logger.log(severity, pre + msg_stripped + post);
        }

        if (!should_log)
            return;

        if (to_console)
            console->log(severity, original_msg);

        // Reset records if clear time has been exceeded
        if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - last_records_clear) > clear_rate)
        {
            clear_records();
        }
    }

    /**
     * Triangle Microworks has its own diagnostic callbacks. If we want to log them using our own style, we need
     * a callback function. This will add the TMW diagnostic callbacks to our own log files and will print them to the
     * consol with the same format of the rest of our console messages.
     * @param pAnlzId A structure consisting of TMWDIAG_ID sourceId, TMWCHNL *pChannel, TMWSESN *pSession, TMWSCTR *pSector, and TMWDTIME time
     * @param pString TMWTYPES_CHAR * of the diagnostic message to be added to the logs
     */
    void log_TMW_message(const TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_CHAR *pString)
    {
        if (!init)
        {
            return;
        }
        bool should_log = true;
        std::string redundant_msg;
        std::string msg = msg_string(std::string(pString));

        should_log = update_records(msg, redundant_msg);

        if (to_file && msg.length() > 0)
        {
            logger.log(spdlog::level::info, msg);
        }

        if (!should_log)
            return;

        if (to_console && msg.length() > 0)
            console->log(spdlog::level::info, msg);

        // Reset records if clear time has been exceeded
        if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - last_records_clear) > clear_rate)
        {
            clear_records();
        }
    }

    /**
     * Log all stored messages to the specified file.
     * @param fileName The file to save the log to
     */
    void log_it(std::string fileName)
    {
        if (!init)
        {
            return;
        }
        std::string dirAndFile = fmt::format("{}/{}.{}", log_dir, fileName, "log");
        logger.logIt(dirAndFile);
        logger.resetBacktrace(64); // TODO possibly put this in config
    }

    /**
     * Log funcion for test logs. Writes to stderr directly as the logger likely has not been configured
     * @param type The destination type of the log, console or file
     * @param severity The severity level of the log, from trace up to critical. Only Info and Error are typically used
     * @param pre Formatted prestring logged before the message
     * @param msg The formatted message
     */
    void log_test(spdlog::level::level_enum severity, std::string pre, std::string msg, std::string post)
    {
        fprintf(stderr, "%s:%s%s%s", severity_names[severity].c_str(), pre.c_str(), msg.c_str(), post.c_str());
    }

    /**
     * Update records with the message received, adding a new record with the current timestamp if
     * the message has not yet been received during the current redundant_rate period. Otherwise, if
     * a record for the message already exists, simply increment its count.
     * @param msg The current message that is being logged
     * @param redundant_msg String notifying of redundant messages, returned to the caller
     * @return true if the record is new and should be logged.
     */
    bool update_records(std::string msg, std::string &redundant_msg)
    {
        auto existing_record = records.find(msg);
        // Add the record if it does not exist
        if (existing_record == records.end())
        {
            records.insert(std::make_pair(msg, RecordEntry{std::chrono::steady_clock::now(), 0}));
            // No redundant message to add, simply return true
            return true;
        }

        // Block redundant messages during redundancy period
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - existing_record->second.timestamp) <= redundant_rate)
        {
            existing_record->second.count++;
            // There is a redundant message, but it should not be logged yet, simply return false
            return false;
        }

        // Refresh record and return redundancy count alert if non-zero
        existing_record->second.timestamp = std::chrono::steady_clock::now();
        if (existing_record->second.count == 0)
            // The messge is ready to be logged and there has not been a redundant message
            return true;

        // The message is ready to be logged and there has been at least one redundant message that should be included in the log
        redundant_msg = std::string(" seen ") + std::to_string(existing_record->second.count) + " more times in redundant period";
        existing_record->second.count = 0;
        return true;
    }

    /**
     * Clear the record of messages
     */
    void clear_records()
    {
        records.clear();
        last_records_clear = std::chrono::steady_clock::now();
    }

    /**
     * @brief Adding file func line to log in json simulated format.
     *
     * @param file will be file macro
     * @param func will be func macro
     * @param line will be line macro
     * @return std::string <- json acceptable build of the fields above.
     */
    std::string pre_string(const char *file, const char *func, const int line)
    {
        // msg
        char msgBuf[500]; // messages must be in this size. Is this too small???
        // char consoleBuf[500];

        // trim file to just the file not the whole path
        std::string trim(file);
        size_t last = trim.find_last_of('/');
        if (last != std::string::npos)
        {
            trim = trim.substr(last + 1);
        }

        // wacky newlines and tabs needed to mimic pretty print of go_flexgen/logger
        snprintf(msgBuf, 500, "\"File\": \"%s\", \"Func\": \"%s\", \"Line\": %d, \"MSG\": \"", trim.c_str(), func, line); // for printing to file
        // snprintf (consoleBuf, 500, "\n\tCaller: %s:%d\n\tFunc: %s\n\tMessage: ", trim.c_str(), line, func); // for printing to console

        return std::string(msgBuf);
    }

    /**
     * Close the quoted message
     */
    std::string post_string()
    {
        return std::string("\"");
    }

    /**
     * Convert a configuration int to spdlog log level:
     * @param severity the log level to set
     * @return the spdlog level
     */
    spdlog::level::level_enum severity_to_level(int severity)
    {
        switch (severity)
        {
        case -1:
            return spdlog::level::trace;
            break;
        case 0:
            return spdlog::level::debug;
            break;
        case 1:
            return spdlog::level::info;
            break;
        case 2:
            return spdlog::level::warn;
            break;
        case 3:
            return spdlog::level::err;
            break;
        case 4:
            return spdlog::level::critical;
            break;
        // spdlog does not have a level higher than critical,
        // so map both panic and fatal levels to critical
        case 5:
            return spdlog::level::critical;
            break;
        default:
            return spdlog::level::info;
            break;
        }
    }
}