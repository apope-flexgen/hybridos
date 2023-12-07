# Flexgen Logger Documentation

## Overview

The Flexgen Logger is a comprehensive logging solution that provides support for both console and file logging. It includes features like a custom timestamp based on the application's runtime, managing redundant logs, and dynamic logging level configurations.

## Features

1. **Console Logging**: Log messages directly to the console.
2. **File Logging**: Write log messages to a specified file.
3. **Custom Elapsed Time Formatting**: A custom formatter for elapsed time since the start of the logger.
4. **Redundancy Management**: Prevent redundant log messages within a specified time period.
5. **Custom Severity Levels**: Assign custom severity levels and their string representation for log messages.
6. **Configurable Log Directory**: Specify the directory where log files should be stored.

## How to Use

### Initialization

To initialize the logger, use the `Logging::Init` function. This function requires the name of the module and the command-line arguments to set up the logger:

```cpp
Logging::Init("ModuleName", argc, argv);
```

### Logging Messages

Use the defined macros for logging messages with different severity levels:

- **Informational Messages**:
  ```cpp
  FPS_INFO_LOG("This is an informational message.");
  ```

- **Debug Messages**:
  ```cpp
  FPS_DEBUG_LOG("Debugging message: Value of x is %d", x);
  ```

- **Warning Messages**:
  ```cpp
  FPS_WARNING_LOG("Warning! The configuration might be invalid.");
  ```

- **Error Messages**:
  ```cpp
  FPS_ERROR_LOG("Error! Failed to open the file.");
  ```

- **Test Messages**:
  ```cpp
  FPS_TEST_LOG("Test message for debugging.");
  ```

### Saving Logs

To save the current log buffer to a file, use:

```cpp
FPS_LOG_IT("filename_without_extension");
```

This will save the logs to a file named "filename_without_extension.txt" in the pre-configured log directory.

## Customization

### Redundancy Management

The logger maintains a record of the messages that have been logged recently. This helps in avoiding redundant log messages in a short time span. You can set the `redundant_rate` variable to specify the minimum time difference required between two identical log messages.

### Custom Severity Levels

You can map custom severity levels to their string representation using the `severity_names` map:

```cpp
std::map<spdlog::level::level_enum, std::string> severity_names;
```

## Conclusion

The Flexgen Logger offers a powerful yet simple interface to handle various logging needs. With support for both console and file-based logging, managing log messages, and custom configurations, it provides a robust solution for application-level logging.