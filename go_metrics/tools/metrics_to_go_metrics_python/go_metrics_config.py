import json

class Meta:
    def __init__(self, publishRate=1000, name="go_metrics", log_file=None, debug=False, debug_inputs=[], debug_outputs=[], debug_filters=[]):
        self.publishRate = publishRate
        self.name = name
        self.log_file = log_file
        self.debug = debug
        self.debug_inputs = debug_inputs
        self.debug_outputs = debug_outputs
        self.debug_filters = debug_filters
    def __str__(self):
        return_str = "{"
        return_str += f'"publishRate":{self.publishRate},'
        return_str += f'"name":"{self.name}",'
        if self.log_file is not None:
            return_str += f'"log_file":{self.log_file},'
        return_str += f'"debug":{str(self.debug).lower()},'
        return_str += '"debug_inputs":[],'
        return_str += '"debug_outputs":[],'
        return_str += '"debug_filters":[]}'
        return return_str

class Input:
    def __init__(self, key, uri, _type, default=None, attributes=None, method=None):
        self.name = key
        self.type = _type
        self.uri = uri
        self.default = default
        self.attributes = attributes
        self.method = method

    def __str__(self):
        return_str = "{"
        return_str += f'"uri": "{self.uri}", "type": "{self.type}"'
        if self.default is not None:
            if self.type == "string" or self.type == "bitfield_string":
                return_str += f', "default": "{self.default}"'
            elif  self.type == "bool":
                 return_str += f', "default": {str(self.default).lower()}'
            else:
                 return_str += f', "default": {self.default}'
        if self.attributes is not None:
            return_str += ', "attributes": ['
            is_first = True
            for attribute in self.attributes:
                if is_first:
                    is_first = False
                else:
                    return_str += ','
                return_str += f'"{attribute}"'
            return_str += ']'
        if self.method is not None:
            return_str += f', "method": "{self.method}"'
        return_str += "}"
        return return_str

class Output:
    def __init__(self, key, uri, flags=None, attributes=None, publishRate=None, name=None, bitfield=None, enum=None):
        self.key = key
        self.uri = uri
        self.flags = flags
        self.attributes = attributes
        self.publishRate = publishRate
        self.name = name
        self.bitfield = bitfield
        self.enum = enum

    def __str__(self):
        return_str = "{"
        if self.name != None:
            return_str += f'"name": "{self.name}", '
        return_str += f'"uri": "{self.uri}"'
        if self.flags is not None and len(self.flags) > 0:
            return_str += ', "flags": ['
            is_first = True
            for flag in self.flags:
                if is_first:
                    is_first = False
                else:
                    return_str += ','
                return_str += f'"{flag}"'
            return_str += ']'
        if self.attributes is not None and len(self.attributes) > 0:
            return_str += ', "attributes": {'
            is_first = True
            for attribute, value in self.attributes.items():
                if is_first:
                    is_first = False
                else:
                    return_str += ','
                if isinstance(value, str):
                    return_str += f'"{attribute}":"{value}"'
                elif isinstance(value, bool):
                    return_str += f'"{attribute}":{str(value).lower()}'
                else:
                    return_str += f'"{attribute}":{value}'
            return_str += '}'
        if self.publishRate is not None:
            return_str += f', "publishRate": "{self.publishRate}"'
        if self.bitfield is not None:
            return_str += f', "bitfield": ['
            is_first = True
            for bit in self.bitfield:
                if is_first:
                    is_first = False
                else:
                    return_str += ','
                return_str += f'"{bit}"'
            return_str += ']'
        return_str += "}"
        return return_str

class MetricsExpression:
    def __init__(self, _type, outputs, expression, id=None):
        self.type = _type
        self.outputs = outputs
        self.expression = expression
        self.id = id

    def __str__(self):
        return_str = "{"
        if self.id is not None:
            return_str += f'"id":"{self.id}",'
        return_str += f'"type":"{self.type}",'
        return_str += f'"outputs": ['
        is_first = True
        for output in self.outputs:
            if is_first:
                is_first = False
            else:
                return_str += ','
            return_str += f'"{output}"'
        return_str += '], '
        return_str += f'"expression":"{self.expression}"'
        return_str += "}"
        return return_str

class GoMetrics:
    def __init__(self, meta=Meta(), inputs={}, outputs={}, metrics=[]):
        self.meta = meta
        self.inputs = inputs
        self.outputs = outputs
        self.metrics = metrics

    def __str__(self):
        return_str = "{\"meta\":"
        return_str += str(self.meta)
        return_str += ",\"inputs\":{"
        is_first = True
        for input_key, value in self.inputs.items():
            if is_first:
                is_first = False
            else:
                return_str += ","
            return_str += f'"{input_key}":'
            return_str += str(value)
        return_str += "},\"outputs\":{"
        is_first = True
        for output_key, value in self.outputs.items():
            if is_first:
                is_first = False
            else:
                return_str += ","
            return_str += f'"{output_key}":'
            return_str += str(value)
        return_str += "},\"metrics\":["
        is_first = True
        for metric in self.metrics:
            if is_first:
                is_first = False
            else:
                return_str += ","
            return_str += str(metric)
        return_str += "]}"
        return return_str