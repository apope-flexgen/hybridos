# Metrics To Go_Metrics
## Setup
In your command prompt (any environment should work), type the following:
```
python -m pip install -r requirements.txt
```
Ensure that all dependencies are installed correctly.

(Replace `python` with `python3` if your environment dictates it.)

## Running the script
Run the script using the following command
```
python <path_to_python_script_folder>/metrics_to_go_metrics.py -i <input_file> -o <output_file>
```
(Replace `python` with `python3` if your environment dictates it.)

If you do not specify an output file, the default will be the input filename, but substituting the last instance of `"metrics"` with `"go_metrics"`. This will not create any folders that are necessary to save the file in the correct location, so you will need to do that in advance.

## Disclaimer
This is largely untested. It should be an improvement over `metrics_to_gometrics.go` (the original Go version of this script), but it may not be a perfect replica of metrics behavior. I have done my best to provide warnings when behavior is unreproducible or ambiguous, but that may not be the case in every situation. It is important to check input types, metrics output types, and bitfields, especially.