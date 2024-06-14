def generate_getters_setters(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()

    with open(output_file, 'w') as f:
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            parts = line.split()
            if len(parts) < 2:
                continue
            data_type = parts[0]
            variable_name = parts[1].rstrip(';')
            setter_name = 'set' + variable_name.capitalize()
            getter_name = 'get' + variable_name.capitalize()

            # Write getter
            f.write(f"{data_type} {getter_name}() override {{ return {variable_name}; }}\n")

            # Write setter
            f.write(f"void {setter_name}({data_type} {variable_name}_) override {{ {variable_name} = {variable_name}_; }}\n\n")

def generate_virtual_getters_setters(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()

    with open(output_file, 'w') as f:
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            parts = line.split()
            if len(parts) < 2:
                continue
            data_type = parts[0]
            variable_name = parts[1].rstrip(';')
            setter_name = 'set' + variable_name.capitalize()
            getter_name = 'get' + variable_name.capitalize()

            # Write getter
            f.write(f"virtual {data_type} {getter_name}() = 0;\n")

            # Write setter
            f.write(f"virtual void {setter_name}({data_type} {variable_name}_) = 0;\n\n")


if __name__ == '__main__':
    generate_getters_setters('input.txt', 'getters_and_setters.txt')
    generate_virtual_getters_setters("input.txt", "virtual_getters_and_setters.txt")