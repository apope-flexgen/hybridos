# PowerCloud FIMS Format

The following rules must be followed by any FIMS message which is processed by PowerCloud.

## Message URI

* Every message collected from the same URI must have the same set of field names, and corresponding field names must be associated with values which are of the same type.

## Message Method

* The message's method must be either `pub` or `post`. Messages with any other method are ignored.

## Message Body

* For the syntax of JSON, see (https://www.json.org).
* The message body must be of one of the following forms:
    * A json object
        ```json
        {
            "<field_name_1>": <field_value_1>,
            "<field_name_2>": <field_value_2>,
            ...
            "<field_name_N>": <field_value_N>
        }
        ```
    * A json object with a single "value" field:
        ```json
        {
            "value": <field_value>
        }
        ```
        In this case, the field_name comes from the last fragment of the URI rather than being "value".
    * A raw json value:
        ```json
        <field_value>
        ```
        In this case, the field_name comes from the last fragment of the URI.
* Each of the field values must be one of the following types:
    * **A naked value:**\
    A naked value can be any of the following types:
        * A JSON number that can be expressed as a float64 (NaN, Inf, and -Inf are not supported).
        * A JSON boolean.
        * A JSON string.
        * A JSON array of objects, where each object has a "value" key that holds an integer and a "string" key that holds a string. Each object may contain additional keys, but those keys will be ignored. For clarity, the format of the array can be written as:
            ```json
            [
                {
                    "value": <integer_value_1>,
                    "string": <string_value_1>,
                    ...
                },
                {
                    "value": <integer_value_2>,
                    "string": <string_value_2>,
                    ...
                },
                ...
                {
                    "value": <integer_value_N>,
                    "string": <string_value_N>,
                    ...
                }
            ]
            ```
            * When an array of objects is processed, it is flattened into separate string fields of the following form. Therefore, be careful that your field names do not conflict with each other once the arrays are flattened.
                ```json
                "<field_name><integer_value_1>": <string_value_1>,
                "<field_name><integer_value_2>": <string_value_2>,
                ...
                "<field_name><integer_value_N>": <string_value_N>,
                ```
    * **A clothed value:**\
    A clothed value is a JSON object with a member named `"value"`. The value of the `"value"` member must be a naked value.
        ```json
        {
            "value": <naked_value>,
            ...
        }
        ```
    * **A UI control value:**\
    A UI control value is a JSON object. It must either have no member named `"value"`, or it must have a member named `"ui_type"` whose value is the string `"control"`. Any field with a UI control value is ignored by PowerCloud.
        ```json
        {
            "ui_control_example_1": true
        },
        {
            "ui_control_example_2": true,
            "value": 1,
            "ui_type": "control"
        },
        {
            "clothed_example_1": true,
            "value": 1,
            "ui_type": "fault"
        }
        ```
* Any field named `"timestamp"` (capitalization does not matter) will be ignored by PowerCloud.
* The order in which JSON object members appear does not matter to PowerCloud.
* Any JSON string, whether it be a member name or a member value, cannot include any of the following characters:
    * `;`
    * `\t`
    * `\n`
* The field name `"ftd_group"` is reserved. There cannot be a field with the name `"ftd_group"`
* JSON null values are not allowed.

## Errors

* Whenever PowerCloud processes a message which breaks one or more of these rules, an error should be logged.
* Whenever the rules state that some data is ignored by PowerCloud, there is no error log associated with that data being ignored.

## Special Messages

* PowerCloud will always listen to `/cops/summary`, though it won't archive any messages from `/cops/summary` unless configured to do so. Instead, it attempts to extract a `"status"` string field from the COPS publish and uses that to determine the controller status.