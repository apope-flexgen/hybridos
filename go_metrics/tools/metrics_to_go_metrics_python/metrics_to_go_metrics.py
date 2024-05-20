import argparse
import json
from compact_json import Formatter, EolStyle
import metrics_config as metrics
import go_metrics_config as go_metrics

def rreplace(s, old, new, occurrence):
    li = s.rsplit(old, occurrence)
    return new.join(li)

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input_file', help='Input file path')
parser.add_argument('-o', '--output_file', help='Output file path')
args = parser.parse_args()
if args.input_file is None:
    print("Need input file for metrics-to-go_metrics conversion. Supply using -i")
    quit(-1)

input_file = args.input_file

if args.output_file is None:
    output_file = rreplace(input_file, "metrics", "go_metrics", 1)
else:
    output_file = args.output_file

metrics_config = metrics.Metrics()
metrics_config.load_config(input_file)
go_metrics_config = go_metrics.GoMetrics()

# process Meta
go_metrics_config.meta = go_metrics.Meta()
go_metrics_config.meta.publishRate = metrics_config.publishRate

# process Inputs, Outputs, and Expressions
for publishUri in metrics_config.publishUris:
    for metric in publishUri.metrics:
        metric.extract_metrics_inputs(go_metrics_config)
        metric.extract_metrics_outputs(go_metrics_config)
        metric.translate_metrics_expression(go_metrics_config)

formatter = Formatter()
formatter.indent_spaces = 4
formatter.max_inline_complexity = 10
formatter.json_eol_style = EolStyle.LF
formatter.align_expanded_property_names = False
formatter.max_inline_length=1000
formatter.table_dict_minimum_similarity = 0
formatter.align_expanded_property_names = True


with open(output_file, 'w', encoding='utf-8') as file:
    file.write(str(go_metrics_config))

with open(output_file, 'r', encoding='utf-8') as file:
    json_obj = json.load(file)
    formatter.dump(json_obj, output_file=output_file, newline_at_eof=True)