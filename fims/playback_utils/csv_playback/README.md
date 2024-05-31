## Usage

Reads all of the columns from input.csv defined in the command_columns in the config and sends the values to the command_uri associated with each column. Also sends periodic gets to the feedback_uris in feedback_columns and saves them to another feedback_file with columns as specified.

```
python3 csvPlayback.py /path/to/input.csv /path/to/csvPlayback_config.json
```