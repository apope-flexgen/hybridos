"""
This Python module defines classes related to a metrics system.

Classes:
- Input: Represents an input associated with a metrics object.
- Output: Represents an output associated with a metrics object.
- MetricsObject: Represents a metrics object containing inputs, outputs, and other attributes.
- PublishUri: Represents a URI associated with publishing metrics.
- Metrics: Represents a collection of metrics with publishing and listening rates.

Each class provides functionality to load configurations from JSON objects and manage metrics-related data.
"""

import json
import go_metrics_config as gm

class Input:
    """
    Represents an input associated with a metrics object.

    Attributes:
    - publishUri_index (int): Index of the parent PublishUri.
    - metrics_index (int): Index of the parent MetricsObject.
    - index (int): Index of the current Input.
    - uri (str): URI of the input.
    - id (str): Identifier of the input.
    """

    publishUri_index = 0
    metrics_index = 0
    index = 0
    all_input_ids = []
    current_input_id_index = {}

    def __init__(self, publishUri_index, metrics_index, uri, id):
        """
        Initializes an Input instance.

        Args:
        - publishUri_index (int): Index of the parent PublishUri.
        - metrics_index (int): Index of the parent MetricsObject.
        - uri (str): URI of the input.
        - id (str): Identifier of the input.
        """
        self.uri = uri
        self.id = id

        # Increment the index if the parent publishUri_index or metrics_index changes
        if publishUri_index != Input.publishUri_index or \
            metrics_index != Input.metrics_index:
            MetricsObject.publishUri_index = publishUri_index
            MetricsObject.metrics_index = metrics_index
            MetricsObject.index = 0
        self.publishUri_index = Input.publishUri_index
        self.metrics_index = Input.metrics_index
        self.index = Input.index
        Input.index += 1

    def load_config(self, json_config):
        """
        Loads Input configuration from a JSON object.

        Args:
        - json_config (dict): JSON object containing Input configuration.

        Returns:
        - bool: True if configuration loaded successfully, False otherwise.
        """
        if isinstance(json_config, dict):
            if "uri" in json_config:
                if isinstance(json_config["uri"], str):
                    self.uri = json_config["uri"]
                else:
                    print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                          + f" Input [{self.index}]: \"uri\" is not a string")
                    return False
            else:
                print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                      + f" Input [{self.index}]: missing \"uri\" field")
                return False

            if "id" in json_config:
                if isinstance(json_config["id"], str):
                    self.id = json_config["id"]
                    Input.all_input_ids.append(self.id)
                else:
                    print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                          + f" Input [{self.index}]: \"id\" is not a string")
                    return False
            else:
                print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                      + f" Input [{self.index}]: missing \"id\" field")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                  + f" Input [{self.index}]: not a JSON object")
            return False

        return True

class Output:
    """
    Represents an output associated with a metrics object.

    Attributes:
    - publishUri_index (int): Index of the parent PublishUri.
    - metrics_index (int): Index of the parent MetricsObject.
    - index (int): Index of the current Output.
    - uri (str): URI of the output.
    - id (str): Identifier of the output.
    """

    publishUri_index = 0
    metrics_index = 0
    index = 0
    all_output_ids = []
    current_output_id_index = {}

    def __init__(self, publishUri_index, metrics_index, uri, id):
        """
        Initializes an Output instance.

        Args:
        - publishUri_index (int): Index of the parent PublishUri.
        - metrics_index (int): Index of the parent MetricsObject.
        - uri (str): URI of the output.
        - id (str): Identifier of the output.
        """
        self.uri = uri
        self.id = id

        # Increment the index if the parent publishUri_index or metrics_index changes
        if publishUri_index != Output.publishUri_index or \
            metrics_index != Output.metrics_index:
            MetricsObject.publishUri_index = publishUri_index
            MetricsObject.metrics_index = metrics_index
            MetricsObject.index = 0
        self.publishUri_index = Output.publishUri_index
        self.metrics_index = Output.metrics_index
        self.index = Output.index
        Output.index += 1

    def load_config(self, json_config):
        """
        Loads Output configuration from a JSON object.

        Args:
        - json_config (dict): JSON object containing Output configuration.

        Returns:
        - bool: True if configuration loaded successfully, False otherwise.
        """
        if isinstance(json_config, dict):
            if "uri" in json_config:
                if isinstance(json_config["uri"], str):
                    self.uri = json_config["uri"]
                else:
                    print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                          + f" Output [{self.index}]: \"uri\" is not a string")
                    return False
            else:
                print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                      + f" Output [{self.index}]: missing \"uri\" field")
                return False

            if "id" in json_config:
                if isinstance(json_config["id"], str):
                    self.id = json_config["id"]
                    Output.all_output_ids.append(self.id)
                else:
                    print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                          + f" Output [{self.index}]: \"id\" is not a string")
                    return False
            else:
                print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                      + f" Output [{self.index}]: missing \"id\" field")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: metrics[{self.metrics_index}]:"
                  + f" Output [{self.index}]: not a JSON object")
            return False

        return True

class MetricsObject:
    """
    Represents a metrics object with inputs, operation, and other attributes.

    Attributes:
    - publishUri_index (int): Index of the parent PublishUri.
    - index (int): Index of the current MetricsObject.
    - inputs (list): List of Input objects.
    - operation (str): Operation performed by the metrics object.
    - id (str): Identifier of the metrics object.
    - outputs (list): List of Output objects.
    - param (dict): Additional parameters for the metrics object.
    - name (str): Name of the metrics object.
    - scale (str): Scale of the metrics object.
    - unit (str): Unit of measurement for the metrics object.
    - ui_type (str): User interface type for visualization.
    - type (str): Type of the metrics object.
    - options (dict): Additional options for the metrics object.
    """

    publishUri_index = 0
    index = 0
    float_inputs = ["max", "min", "product", "average","integrate","runtime","rss","unicompare","compare","compareand","compareor","quadtosigned","signedtoquad"]
    uint_inputs = ["and","bitfield","splitMillisecondsTo32BitInts","reassembleMillisecondsFrom32BitInts","compareMillisecondsToCurrentTime","millisecondsToRFC3339"]
    bool_inputs = ["or","srff","pulse"]
    bitfield_int_inputs = ["bitfieldpositioncount"]
    ambiguous_inputs = ["sum", "add", "echo","select","length","selectorn","combineInputsToArray", "selectn"]
    float_outputs = ["max", "min", "product", "average","integrate","runtime","rss","unicompare","quadtosigned","signedtoquad"]
    uint_outputs = ["and","bitfield","splitMillisecondsTo32BitInts","reassembleMillisecondsFrom32BitInts", "bitfieldpositioncount","length","selectorn","currentTimeMilliseconds"]
    bool_outputs = ["or","srff","pulse","compare","compareand","compareor","compareMillisecondsToCurrentTime"]
    string_outputs = ["millisecondsToRFC3339"]
    ambiguous_outputs = ["sum", "add", "echo","select","combineInputsToArray", "selectn"]

    def __init__(self, publishUri_index, publishUri_item, inputs, operation, id="undefined", outputs=None, param=None, name=None, scale=None, unit=None, ui_type=None, _type=None, options=None):
        """
        Initializes a MetricsObject instance.

        Args:
        - publishUri_index (int): Index of the parent PublishUri.
        - inputs (list): List of Input objects.
        - operation (str): Operation performed by the metrics object.
        - id (str, optional): Identifier of the metrics object. Defaults to "undefined".
        - outputs (list, optional): List of Output objects. Defaults to None.
        - param (dict, optional): Additional parameters for the metrics object. Defaults to None.
        - name (str, optional): Name of the metrics object. Defaults to None.
        - scale (str, optional): Scale of the metrics object. Defaults to None.
        - unit (str, optional): Unit of measurement for the metrics object. Defaults to None.
        - ui_type (str, optional): User interface type for visualization. Defaults to None.
        - _type (str, optional): Type of the metrics object. Defaults to None.
        - options (dict, optional): Additional options for the metrics object. Defaults to None.
        """
        self.naked = publishUri_item.naked
        self.inputs = inputs
        self.operation = operation
        self.id = id
        self.outputs = outputs
        self.param = param
        self.name = name
        self.scale = scale
        self.unit = unit
        self.ui_type = ui_type
        self.type = _type
        self.options = options
        self.published_output = None
        self.parent = publishUri_item

        # Increment the index if the parent publishUri_index changes
        if publishUri_index != MetricsObject.publishUri_index:
            MetricsObject.publishUri_index = publishUri_index
            MetricsObject.index = 0
        self.publishUri_index = MetricsObject.publishUri_index
        self.index = MetricsObject.index
        MetricsObject.index += 1

    def load_config(self, json_config):
        """
        Loads MetricsObject configuration from a JSON object.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: True if configuration loaded successfully, False otherwise.
        """
        if isinstance(json_config, dict):
            if not self._load_inputs(json_config):
                return False

            if not self._load_operation(json_config):
                return False

            if not self._load_id(json_config):
                return False

            if not self._load_outputs(json_config):
                return False

            if not self._load_param(json_config):
                return False

            self._load_optional_fields(json_config)

            if self.parent:
                self.published_output = Output(self.publishUri_index, self.index, self.parent.uri, self.id)
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                + f" [{self.index}]: no published output")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                + f" [{self.index}]: not a JSON object")
            return False

        return True

    def _load_inputs(self, json_config):
        """
        Loads the 'inputs' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: True if 'inputs' loaded successfully, False otherwise.
        """
        if "inputs" in json_config:
            self.inputs = []
            if isinstance(json_config["inputs"], list):
                for input_config in json_config["inputs"]:
                    input_obj = Input(self.publishUri_index, self.index, "", "")
                    ok = input_obj.load_config(input_config)
                    if ok:
                        self.inputs.append(input_obj)
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                    + f" [{self.index}]: \"inputs\" is not an array")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                + f" [{self.index}]: missing \"inputs\" field")
            return False

        return True

    def _load_operation(self, json_config):
        """
        Loads the 'operation' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: True if 'operation' loaded successfully, False otherwise.
        """
        if "operation" in json_config:
            if isinstance(json_config["operation"], str):
                self.operation = json_config["operation"]
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                    + f" [{self.index}]: \"operation\" is not a string")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                + f" [{self.index}]: missing \"operation\" field")
            return False

        return True

    def _load_id(self, json_config):
        """
        Loads the 'id' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: True if 'id' loaded successfully, False otherwise.
        """
        if "id" in json_config:
            if isinstance(json_config["id"], str):
                self.id = json_config["id"]
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                    + f" [{self.index}]: \"id\" is not a string")
                return False
        else:
            print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                + f" [{self.index}]: missing \"id\" field")
            return False

        return True

    def _load_outputs(self, json_config):
        """
        Loads the 'outputs' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: True if 'outputs' loaded successfully, False otherwise.
        """
        self.outputs = []
        if "outputs" in json_config:
            if isinstance(json_config["outputs"], list):
                for output_config in json_config["outputs"]:
                    output_obj = Output(self.publishUri_index, self.index, "", "")
                    ok = output_obj.load_config(output_config)
                    if ok:
                        self.outputs.append(output_obj)
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                    + f" [{self.index}]: \"outputs\" is not an array")
                return False

        return True

    def _load_param(self, json_config):
        """
        Loads the 'param' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.

        Returns:
        - bool: Always returns True as 'param' is optional.
        """
        if "param" in json_config:
            if isinstance(json_config["param"], dict):
                self.param = json_config["param"]
            else:
                print(f"publishUris[{self.publishUri_index}]: Invalid metrics object at index" \
                    + f" [{self.index}]: \"param\" is not a JSON object; defaulting to None")
                self.param = None

        return True

    def _load_optional_fields(self, json_config):
        """
        Loads optional fields from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing MetricsObject configuration.
        """
        self.name = json_config.get("name", None)
        self.scale = json_config.get("scale", None)
        self.unit = json_config.get("unit", None)
        self.ui_type = json_config.get("ui_type", None)
        self.type = json_config.get("type", None)
        self.options = json_config.get("options", None)

    def extract_metrics_inputs(self, go_metrics):
        """
        Extracts and modifies input data for go_metrics collection.

        Parameters:
        - go_metrics: An object to store input data.

        This method iterates through the inputs, constructs a unique identifier if necessary,
        and adds them to the go_metrics object with their associated metadata.
        """
        for input in self.inputs:
            original_id = input.id
            full_uri = input.uri + "/" + input.id
            _type = self.infer_input_type(input)
            _default = self.get_default_value(_type)
            if Input.all_input_ids.count(input.id) > 1:
                if input.id in Input.current_input_id_index.keys():
                    input.id = input.id + "_" + str(Input.current_input_id_index[input.id])
                    Input.current_input_id_index[original_id] += 1
                else:
                    input.id = input.id + "_1"
                    Input.current_input_id_index[original_id] = 2
            go_metrics.inputs[input.id] = gm.Input(original_id, full_uri, _type, default=_default)

    def extract_metrics_outputs(self, go_metrics):
        """
        Extracts and modifies output data for go_metrics collection based on various conditions.

        Parameters:
        - go_metrics: An object to store modified output data.

        The method processes both individual and set outputs, applying unique identifiers and
        handling specific attributes and flags based on the operation mode.
        """
        original_id = self.published_output.id
        flags = []
        attributes = {}
        if self.name is not None:
            attributes["name"] = self.name
        if self.scale is not None:
            attributes["scale"] = self.scale
        if self.unit is not None:
            attributes["unit"] = self.unit
        if self.ui_type is not None:
            attributes["ui_type"] = self.ui_type
        if self.type is not None:
            attributes["type"] = self.type
        if self.options is not None:
            attributes["options"] = self.options
        if not self.naked:
            flags.append("clothed")
        if Output.all_output_ids.count(self.published_output.id) > 1:
            if self.published_output.id in Output.current_output_id_index:
                self.published_output.id = self.published_output.id + "_" + str(Output.current_output_id_index[self.published_output.id])
                Output.current_output_id_index[original_id] += 1
            else:
                self.published_output.id = self.published_output.id + "_1"
                Output.current_output_id_index[original_id] = 2
        go_metrics.outputs[self.published_output.id] = gm.Output(original_id, self.published_output.uri, flags, attributes)
        if self.operation == "bitfield" and "string" in self.param:
            go_metrics.outputs[self.published_output.id].flags.append("bitfield")
            go_metrics.outputs[self.published_output.id].bitfield = self.param["string"]
        if self.operation == "bitfield" and "position" in self.param and isinstance(self.param["position"], list):
            for i, string in enumerate(go_metrics.outputs[self.published_output.id].bitfield):
                if i < len(self.param["position"]):
                    go_metrics.outputs[self.published_output.id].bitfield[i] = {"value":self.param["position"][i], "string":string}

        ## now handle all of the outputs that are sent out as sets
        for output in self.outputs:
            original_id = output.id
            flags = ["direct_set", "lonely","clothed"]
            attributes = {}
            if self.name is not None:
                attributes["name"] = self.name
            if self.scale is not None:
                attributes["scale"] = self.scale
            if self.unit is not None:
                attributes["unit"] = self.unit
            if self.ui_type is not None:
                attributes["ui_type"] = self.ui_type
            if self.type is not None:
                attributes["type"] = self.type
            if self.options is not None:
                attributes["options"] = self.options
            if not self.naked and "clothed" not in flags:
                flags.append("clothed")
            if Output.all_output_ids.count(output.id) > 1:
                if output.id in Output.current_output_id_index:
                    output.id = output.id + "_" + str(Output.current_output_id_index[output.id])
                    Output.current_output_id_index[output.id] += 1
                else:
                    output.id = output.id + "_1"
                    Output.current_output_id_index[output.id] = 2
            go_metrics.outputs[output.id] = gm.Output(original_id, output.uri, flags, attributes)
            if self.operation == "bitfield" and "string" in self.param:
                go_metrics.outputs[output.id].flags.append("bitfield")
                go_metrics.outputs[output.id].bitfield = self.param["string"]

    def infer_input_type(self, input):
        """
        Infers the type of a given input based on the metrics function and the input name.

        Parameters:
        - input: The input object whose type is to be inferred.

        Returns: A string representing the inferred type of the input.
        """
        if "bool" in input.id or "flag" in input.id:
            return "bool"
        if "float" in input.id or "_cmd" in input.id:
            return "float"
        if "rate" in input.id:
            return "int"
        if "input_source_status" in input.id or "name" in input.id or \
            "site_state" in input.id or "site_status" in input.id:
                return "string"
        if self.operation in MetricsObject.float_inputs:
            return "float"
        if self.operation in MetricsObject.uint_inputs:
            return "uint"
        if self.operation in MetricsObject.bool_inputs:
            return "bool"
        if self.operation in MetricsObject.bitfield_int_inputs:
            return "bitfield_int"
        return "float"

    def infer_output_type(self):
        """
        Infers the type of a given function's output based on the metrics function and the output names.

        Returns: A string representing the inferred type of the output.
        """
        for output in self.outputs:
            if "bool" in output.id or "flag" in output.id:
                return "bool"
            if "float" in output.id or "_cmd" in output.id:
                return "float"
            if "rate" in output.id:
                return "int"
            if "input_source_status" in output.id or "name" in output.id or \
                "site_state" in output.id or "site_status" in output.id:
                return "string"
        if self.operation in MetricsObject.float_outputs:
            return "float"
        if self.operation in MetricsObject.uint_outputs:
            return "uint"
        if self.operation in MetricsObject.bool_outputs:
            return "bool"
        if self.operation in MetricsObject.string_outputs:
            return "string"
        return "float"

    def get_default_value(self, _type):
        """
        Returns a default value based on the specified type.

        Parameters:
        - _type: A string representing the type of data for which a default value is needed.

        Returns:
        - The default value appropriate for the given type. Returns 0.0 for unrecognized types.

        Supported types include:
        - "float": Returns 0.0
        - "string": Returns an empty string ""
        - "int": Returns 0
        - "uint": Returns 0 (unsigned integer)
        - "bool": Returns False (boolean)
        """
        if _type == "float":
            return 0.0
        if _type == "string":
            return ""
        if _type == "int":
            return 0
        if _type == "uint":
            return 0
        if _type == "bool":
            return False
        return 0.0

    def translate_metrics_expression(self, go_metrics):
        """
        Executes a translation method based on the current metrics function.

        This function dynamically selects and calls a method based on the operation mode
        stored in `self.operation`. This approach allows for flexible handling of different
        metric translation needs.

        Parameters:
        - go_metrics: the matching go_metrics config.

        The function constructs the method name by appending 'translate_' to the current operation
        string, then fetches and calls this method from the current object's method dictionary.
        """
        method_name = "translate_" + self.operation
        method = getattr(self, method_name)
        method(go_metrics)

    def translate_max(self, go_metrics):
        str_expression = "Max("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))

    def translate_min(self, go_metrics):
        str_expression = "Min("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
    
    def translate_average(self, go_metrics):
        str_expression = "Average("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))

    def translate_sum(self, go_metrics):
        is_guaranteed_float = False
        str_expression = ""
        if self.param is not None and 'operations' in self.param:
            if "-" in self.param["operations"]:
                is_guaranteed_float = True
            is_first = True
            for idx, op in enumerate(self.param['operations']):
                if is_first:
                    is_first = False
                    if op == "-" and "offset" not in self.param:
                        str_expression += "0"
                    elif op == "+" and "offset" not in self.param:
                        str_expression += self.inputs[idx].id
                        continue
                str_expression += f' {op} {self.inputs[idx].id}'
        else:
            is_first = True
            for input in self.inputs:
                if is_first:
                    is_first = False
                    if self.param is not None and "offset" not in self.param:
                        str_expression += input.id
                        continue
                    elif self.param is None:
                        str_expression += input.id
                        continue
                    else:
                        str_expression += " + " + input.id
                else:
                    str_expression += " + " + input.id
        if self.param is not None and "offset" in self.param:
            str_expression = str(self.param['offset']) + str_expression
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        if is_guaranteed_float:
            go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
        else: # might be a string
            go_metrics.metrics.append(gm.MetricsExpression(self.infer_output_type(), outputs, str_expression,self.id))

    def translate_add(self, go_metrics):
        str_expression = ""
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
                str_expression += input.id
            else:
                str_expression += " + " + input.id

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression(self.infer_output_type(), outputs, str_expression,self.id))

    def translate_product(self, go_metrics):
        str_expression = ""
        if self.param is not None and 'operations' in self.param:
            is_first = True
            for idx, op in enumerate(self.param['operations']):
                if is_first:
                    is_first = False
                    if op == "/" and "gain" not in self.param:
                        str_expression += "1"
                    elif op == "*" and "gain" not in self.param:
                        str_expression += self.inputs[idx].id
                        continue  
                str_expression += f' {op} {self.inputs[idx].id}'
        else:
            is_first = True
            for input in self.inputs:
                if is_first:
                    is_first = False
                    if self.param is not None and "gain" not in self.param:
                        str_expression += input.id
                        continue
                    elif self.param is None:
                        str_expression += input.id
                else:
                    str_expression += " * " + input.id
        if self.param is not None and "gain" in self.param:
            str_expression = str(self.param['gain']) + str_expression
        if self.param is not None and "upper" in self.param:
            str_expression = f'Max({self.param["upper"]}, {str_expression})'
        if self.param is not None and "lower" in self.param:
            str_expression = f'Min({self.param["lower"]}, {str_expression})'
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))

    def translate_echo(self, go_metrics):
        if len(self.inputs) > 0:
            str_expression = self.inputs[0].id
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression(self.infer_output_type(), outputs, str_expression,self.id))

    def translate_integrate(self, go_metrics):
        str_expression = "Integrate("
        if len(self.inputs) > 0:
            str_expression += self.inputs[0].id

        timescale = None
        minuteReset = None
        minuteOffset = None
        paramAbs = None
        numParams = 0
        if self.param:
            if "timescale" in self.param:
                timescale = self.param["timescale"]
                numParams = 1
            if "minuteReset" in self.param:
                minuteReset = self.param["minuteReset"]
                numParams = 2
            if "minuteOffset" in self.param:
                minuteOffset = self.param["minuteOffset"]
                numParams = 3
            if "abs" in self.param:
                paramAbs = self.param["abs"]
                numParams = 4
        if numParams >= 1:
            if timescale is not None:
                str_expression += ", " + str(timescale)
            else:
                str_expression += ",1"
        if numParams >= 2:
            if minuteReset is not None:
                str_expression += ", " + str(minuteReset)
            else:
                str_expression += ",0"
        if numParams >= 3:
            if minuteOffset is not None:
                str_expression += ", " + str(minuteOffset)
            else:
                str_expression += ",0"
        if numParams == 4:
            if paramAbs is not None:
                if paramAbs:
                    str_expression += ",true"
                else: 
                    str_expression += ",false"
            else:
                str_expression += ",false"
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
    
    def translate_and(self, go_metrics):
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        print(f"Metrics \"and\" operation needs additional bitfield information provided for outputs [{outputs}].")
        ## Assume inputs are bitfield or enums [{"value": 5, "string": "blah"}]
        ## by default, we want to look at the "value" field
        str_expression = "CombineBits("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression, self.id))

    def translate_or(self, go_metrics):
        str_expression = "Or("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))
    
    def translate_runtime(self, go_metrics):
        str_expression = "Runtime("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id

        gain = None
        upperLimit = None
        minP = None
        defaultP = None
        numParams = 0
        if self.param:
            if "gain" in self.param:
                gain = self.param["gain"]
                numParams = 1
            if "upperLimit" in self.param:
                upperLimit = self.param["upperLimit"]
                numParams = 2
            if "minP" in self.param:
                minP = self.param["minP"]
                defaultP = minP
                numParams = 4
            if "defaultP" in self.param:
                defaultP = self.param["defaultP"]
                numParams = 4
        if numParams >= 1:
            if gain is not None:
                str_expression += ", " + str(gain)
            else:
                str_expression += ",false"
        if numParams >= 2:
            if upperLimit is not None:
                str_expression += ", " + str(upperLimit)
            else:
                str_expression += ",false"
        if numParams >= 3:
            if minP is not None:
                str_expression += ", " + str(minP)
            else:
                str_expression += ",0"
        if numParams == 4:
            if defaultP is not None:
                if defaultP:
                    str_expression += ", " + str(defaultP)
                else: 
                    str_expression += ", " + str(minP)
            else:
                str_expression += ", 0"
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))

    def translate_bitfield(self, go_metrics):
        str_expression = ""
        is_first = True
        for i, input in enumerate(self.inputs):
            invert = ""
            if self.param is not None and "invertMask" in self.param and (isinstance(self.param["invertMask"], str) or isinstance(self.param["invertMask"], list)):
                if i < len(self.param["invertMask"]):
                    invertVal = self.param["invertMask"][i]
                    if invertVal:
                        invert = "!"
            if is_first:
                is_first = False
                str_expression += f"Pow(2, {i})*Int({invert}Bool({input.id}))"
            else:
                str_expression += " + " + f"Pow(2, {i})*Int({invert}Bool({input.id}))"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression,self.id))
    
    def translate_bitfieldpositioncount(self, go_metrics):
        str_expression = "Sum(Compare("
        position = "0"
        invert = "\\\"==\\\""
        if self.param is not None and "position" in self.param:
            position = self.param["position"]
        if self.param is not None and "invert" in self.param:
            invertVal = self.param["invert"]
            if invertVal:
                invert = "\\\"!=\\\""
        str_expression += f"{invert}, {position}"
        for input in self.inputs:
            str_expression += f", {input.id}"
        str_expression += "))"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression,self.id))
    
    def translate_rss(self, go_metrics):
        str_expression = "Rss("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))

    def translate_unicompare(self, go_metrics):
        str_expression = "Unicompare("
        if self.param and "invert" in self.param:
            invertVal = self.param["invert"]
            if invertVal:
                str_expression += "-"
        if len(self.inputs) > 0:
            str_expression += self.inputs[0].id

        if len(self.inputs) > 1:
            str_expression += f", {self.inputs[1].id}"
        else:
            str_expression += f", 0"
        
        if self.param and "balance" in self.param:
            str_expression += f', {str(self.param["balance"]).lower()}'
        
        str_expression += ")"
        
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
    
    def translate_srff(self, go_metrics):
        str_expression = "Srff("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))
    
    def translate_compare(self, go_metrics):
        operation = "eq"
        if self.param and "op" in self.param:
            operation = self.param["op"]
        if self.param and "operation" in self.param:
            operation = self.param["operation"]
        
        
        if operation == "gt":
            str_expression = "GreaterThan("
        elif operation == "gte":
            str_expression = "GreaterThanOrEqual("
        elif operation == "lt":
            str_expression = "LessThan("
        elif operation == "lte":
            str_expression = "LessThanOrEqual("
        elif operation == "ne":
            str_expression = "NotEqual("
        else:
            str_expression = "Equal("
 
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
                str_expression += input.id
            else:
                str_expression += f", {input.id}"
        
        if self.param and "reference" in self.param:
            str_expression += f', {self.param["reference"]}'
        str_expression += ")"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))
    
    def translate_compareand(self, go_metrics):
        operation = "eq"
        if self.param and "op" in self.param:
            operation = self.param["op"]
        if self.param and "operation" in self.param:
            operation = self.param["operation"]

        if operation == "gt":
            str_expression = "CompareAnd(\\\">\\\""
        elif operation == "gte":
            str_expression = "CompareAnd(\\\">=\\\""
        elif operation == "lt":
            str_expression = "CompareAnd(\\\"<\\\""
        elif operation == "lte":
            str_expression = "CompareAnd(\\\"<=\\\""
        elif operation == "ne":
            str_expression = "CompareAnd(\\\"!=\\\""
        else:
            str_expression = "CompareAnd(\\\"==\\\""

        if self.param and "reference" in self.param:
            str_expression += f', {self.param["reference"]}'
        
        for input in self.inputs:
            str_expression += f", {input.id}"
        
        str_expression += ")"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))

    def translate_compareor(self, go_metrics):
        operation = "eq"
        if self.param and "op" in self.param:
            operation = self.param["op"]
        if self.param and "operation" in self.param:
            operation = self.param["operation"]
        
        if operation == "gt":
            str_expression = "CompareOr(\\\">\\\""
        elif operation == "gte":
            str_expression = "CompareOr(\\\">=\\\""
        elif operation == "lt":
            str_expression = "CompareOr(\\\"<\\\""
        elif operation == "lte":
            str_expression = "CompareOr(\\\"<=\\\""
        elif operation == "ne":
            str_expression = "CompareOr(\\\"!=\\\""
        else:
            str_expression = "CompareOr(\\\"==\\\""

        if self.param and "reference" in self.param:
            str_expression += f', {self.param["reference"]}'
        
        for input in self.inputs:
            str_expression += f", {input.id}"
        
        
        str_expression += ")"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))

    def translate_select(self, go_metrics):
        output_type = None
        true_case = None
        false_case = None
        selector = None
        if len(self.inputs) > 0:
            selector = self.inputs[0].id
        if len(self.inputs) > 1:
            true_case = self.inputs[1].id
        if len(self.inputs) > 2:
            false_case = self.inputs[2].id
        if self.param and "trueCase" in self.param:
            if "falseCase" in self.param:
                false_case = self.param["falseCase"]
                if isinstance(false_case, str):
                    false_case = f'\\\"{false_case}\\\"'
                    output_type = "string"
            else:
                if len(self.inputs) > 1:
                    false_case = self.inputs[1].id
            true_case = self.param["trueCase"]
            if isinstance(true_case, str):
                true_case = f'\\\"{true_case}\\\"'
                output_type = "string"
        elif self.param and "falseCase" in self.param:
            false_case = self.param["falseCase"]
            if isinstance(false_case, str):
                false_case = f'\\\"{false_case}\\\"'
                output_type = "string"
        
        str_expression = f'If({selector}, {true_case}, {false_case})'
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        if output_type is None:
            output_type = self.infer_input_type(self.inputs[1])
        go_metrics.metrics.append(gm.MetricsExpression(output_type, outputs, str_expression,self.id))
    
    def translate_length(self, go_metrics):
        str_expression = "Count("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression,self.id))
    
    def translate_quadtosigned(self, go_metrics):
        str_expression = "QuadToSigned("
        if len(self.inputs) > 0:
            str_expression += self.inputs[0].id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
    
    def translate_signedtoquad(self, go_metrics):
        str_expression = "SignedToQuad("
        if len(self.inputs) > 0:
            str_expression += self.inputs[0].id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("float", outputs, str_expression,self.id))
    
    def translate_pulse(self, go_metrics):
        trigger = None
        reset = None
        invert = ""
        timeout = 0
        if len(self.inputs) > 0:
            trigger = self.inputs[0].id
        if len(self.inputs) > 1:
            reset = self.inputs[1].id
        if self.param and "invert" in self.param:
            invertVal = self.param["invert"]
            if invertVal:
                invert = "!"
        if self.param and "time" in self.param:
            timeout = self.param["time"]
        str_expression = f"{invert}Pulse({trigger}, {reset}, {timeout})"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))
    
    def translate_selectn(self, go_metrics):
        index = None
        if len(self.inputs) > 0:
            index = self.inputs[0].id
        str_expression = f"SelectN(Round({index})"
        for input in self.inputs:
            str_expression += ", " + input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression(self.infer_output_type(), outputs, str_expression,self.id))

    def translate_selectorn(self, go_metrics):
        str_expression = "SelectorN("
        is_first = True
        for input in self.inputs:
            if is_first:
                is_first = False
            else:
                str_expression += ", "
            str_expression += input.id
        str_expression += ")"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression(self.infer_output_type(), outputs, str_expression,self.id))

    def translate_currentTimeMilliseconds(self,go_metrics):
        str_expression = "CurrentTimeMilliseconds()"
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)
        go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression,self.id))
    
    def translate_compareMillisecondsToCurrentTime(self, go_metrics):
        operation = "lt"
        reference = 0

        if self.param and "reference" in self.param:
            reference = self.param["reference"]

        if self.param and "operation" in self.param:
            operation = self.param["operation"]
        
        old_time = None
        if len(self.inputs) > 0:
            old_time = self.inputs[0].id
        
        str_expression = f"CurrentTimeMilliseconds() - {old_time}"

        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)

        if self.param is None:
            go_metrics.metrics.append(gm.MetricsExpression("uint", outputs, str_expression,self.id))
            return

        if operation == "gt":
            str_expression += f" > {reference}"
        elif operation == "gte":
            str_expression += f" >= {reference}"
        elif operation == "lt":
            str_expression += f" < {reference}"
        elif operation == "lte":
            str_expression += f" <= {reference}"
        elif operation == "ne":
            str_expression += f" != {reference}"
        else:
            str_expression += f" == {reference}"

        go_metrics.metrics.append(gm.MetricsExpression("bool", outputs, str_expression,self.id))

    
    def translate_millisecondsToRFC3339(self, go_metrics):
        outputs = []
        for output in self.outputs:
            outputs.append(output.id)
        outputs.append(self.published_output.id)

        if len(self.inputs) > 0:
            str_expression = self.inputs[0].id
        
        if self.param is not None and ("operation" in self.param and self.param["operation"] != "zulu"):
             print("Note that millisecondsToRFC may be an imperfect conversion for non-zulu time.")

        go_metrics.metrics.append(gm.MetricsExpression("string", outputs, str_expression,self.id))

    def translate_combineInputsToArray(self, go_metrics):
        print("combineInputsToArray is unhandled")
    
    def translate_reassembleMillisecondsFrom32BitInts(self, go_metrics):
        print("reassembleMillisecondsFrom32BitInts is unhandled")
    
    def translate_splitMillisecondsTo32BitInts(self, go_metrics):
        print("splitMillisecondsTo32BitInts is unhandled")

class PublishUri:
    """
    A class to represent publishUris array item.

    Attributes:
    - uri (str): The URI for the PublishUri.
    - naked (bool): Flag indicating whether the PublishUri is naked.
    - metrics (list): List of MetricsObject associated with this PublishUri.
    - index (int): Index of the PublishUri instance.

    Note:
    - The 'index' attribute is a class variable that increments with each instance creation.
    """

    index = 0

    def __init__(self, uri, metrics, naked=False):
        """
        Initialize a PublishUri object.

        Args:
        - uri (str): The URI for the PublishUri.
        - metrics (list): List of MetricsObject associated with this PublishUri.
        - naked (bool): Flag indicating whether the PublishUri is naked.
        """
        self.uri = uri
        self.naked = naked
        self.metrics = metrics
        self.index = PublishUri.index
        PublishUri.index += 1

    def load_config(self, json_config):
        """
        Loads PublishUri configuration from a JSON object.

        Args:
        - json_config (dict): JSON object containing PublishUri configuration.

        Returns:
        - bool: True if configuration loaded successfully, False otherwise.
        """
        if isinstance(json_config, dict):
            if "uri" in json_config:
                self.uri = json_config["uri"]
            else:
                print(f"Invalid publishUri at index [{self.index}]: missing \"uri\" field")
                return False

            if not self._load_naked(json_config):
                return False

            if not self._load_metrics(json_config):
                return False
        else:
            print(f"Invalid publishUri at index [{self.index}]: not a JSON object")
            return False

        return True

    def _load_naked(self, json_config):
        """
        Loads the 'naked' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing PublishUri configuration.

        Returns:
        - bool: True if 'naked' loaded successfully, False otherwise.
        """
        if "naked" in json_config:
            self.naked = json_config["naked"]
            if isinstance(self.naked, str):
                if self.naked.lower() == "true":
                    self.naked = True
                elif self.naked.lower() == "false":
                    self.naked = False
                else:
                    self.naked = False
                    print(f"Invalid publishUri at index [{self.index}]: \"naked\" field is" \
                          + " not true/false value; using default of 'false'")
            elif not isinstance(self.naked, bool):
                self.naked = False
                print(f"Invalid publishUri at index [{self.index}]: \"naked\" field is" \
                          + " not true/false value; using default of 'false'")
        else:
            self.naked = False
        return True

    def _load_metrics(self, json_config):
        """
        Loads the 'metrics' attribute from the JSON configuration.

        Args:
        - json_config (dict): JSON object containing PublishUri configuration.

        Returns:
        - bool: True if 'metrics' loaded successfully, False otherwise.
        """
        if "metrics" in json_config:
            self.metrics = []
            if isinstance(json_config["metrics"], list):
                for metrics_object_config in json_config["metrics"]:
                    metrics_object = MetricsObject(self.index, self, [], "", "")
                    ok = metrics_object.load_config(metrics_object_config)
                    if ok:
                        self.metrics.append(metrics_object)
            else:
                print(f"Invalid publishUri at index [{self.index}]: \"metrics\" is not"
                      + " an array")
                return False
        else:
            print(f"Invalid publishUri at index [{self.index}]: Missing \"metrics\" array")
            return False

        return True

class Metrics:
    """
    A class to represent metrics configuration.

    Attributes:
    - publishRate (int): The publish rate of metrics.
    - listenRate (int): The listen rate of metrics.
    - metricsUri (str): The URI of metrics.
    - publishUris (list): A list of PublishUri objects.
    """

    def __init__(self, publishRate=0, listenRate=0, metricsUri="", publishUris=[]):
        """
        Initializes Metrics with default or provided values.

        Args:
        - publishRate (int): The publish rate of metrics.
        - listenRate (int): The listen rate of metrics.
        - metricsUri (str): The URI of metrics.
        - publishUris (list): A list of PublishUri objects.
        """
        self.publishRate = publishRate
        self.listenRate = listenRate
        self.metricsUri = metricsUri # Note: This doesn't actually do anything in metrics
        self.publishUris = publishUris
        self.config = {}

    def load_config(self, config_file_path):
        """
        Loads metrics configuration from a JSON file.

        Args:
        - config_file_path (str): The file path of the JSON configuration file.

        Returns:
        - bool: True if configuration loaded successfully, False otherwise.
        """
        try:
            with open(config_file_path, 'r') as config_file:
                config = json.load(config_file)
                self.config = config
        except FileNotFoundError:
            print("Config file not found.")
            return False
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON: {e}")
            return False

        if isinstance(config, dict):
            if "publishRate" in config:
                self.publishRate = config["publishRate"]
            if "listenRate" in config:
                self.listenRate = config["listenRate"]
            if "metricsUri" in config:
                self.metricsUri = config["metricsUri"]
            if "publishUris" in config:
                self.publishUris = []
                if isinstance(config["publishUris"], list):
                    for publishUri_item in config["publishUris"]:
                        pubUri = PublishUri("", {}, False)
                        ok = pubUri.load_config(publishUri_item)
                        if ok:
                            self.publishUris.append(pubUri)
                else:
                    print("Invalid metrics config: \"publishUris\" is not an array")
                    return False
            else:
                print("Invalid metrics config: Missing \"publishUris\" array")
                return False
        else:
            print("Invalid metrics config: Config is not a JSON object")
            return False

        return True
