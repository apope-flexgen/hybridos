import csv
import os
import subprocess
import time
import argparse
import json

def get_feedback(feedback_uri, feedback_column):
    command = f"fims_send -m get -r /$$ -u /ess{feedback_uri}"
    output = subprocess.check_output(command, shell=True, universal_newlines=True)
    # print(command)
    return output.strip()

def send_command(command_uri, value):
    command = f"fims_send -m set -u /ess{command_uri} -- {value}"
    # print(command)
    subprocess.run(command, shell=True)

def main(file_path, config_path):
    with open(config_path, "r") as config_file:
        config = json.load(config_file)

    feedback_file = config["feedback_file"]
    feedback_columns = [column["feedback_column"] for column in config["feedback_columns"]]

    file_exists = os.path.isfile(feedback_file)
    with open(feedback_file, "a") as out_file:
        writer = csv.writer(out_file)
        if not file_exists:
            writer.writerow(feedback_columns)

        with open(file_path, "r") as csv_file:
            reader = csv.DictReader(csv_file)
            first_iteration = True
            for row in reader:
                if not first_iteration:
                    # Query the underlying system and save feedback data
                    feedback_values = []
                    for feedback_config in config["feedback_columns"]:
                        feedback_uri = feedback_config["feedback_uri"]
                        feedback_column = feedback_config["feedback_column"]
                        value = get_feedback(feedback_uri, feedback_column)
                        feedback_values.append(value)

                    writer.writerow(feedback_values)

                # Execute commands from the current row of the CSV file
                for command_config in config["command_columns"]:
                    column = command_config["column"]
                    command_uri = command_config["command_uri"]
                    value = row[column]
                    send_command(command_uri, value)

                # Sleep for the configured playback period
                time.sleep(config["playback_period"])

                first_iteration = False

    print(f"All lines in {file_path} have been executed. {feedback_file} has been saved and closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file_path", help="Path to the input CSV file")
    parser.add_argument("config_path", help="Path to the configuration JSON file")
    args = parser.parse_args()

    main(args.file_path, args.config_path)