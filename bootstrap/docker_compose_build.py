import re
import os
import yaml

class Network:
    def __init__(self, name, ip_addr, site_number=0):
        self.name = name
        self.ip_addr = ip_addr
        self.site_number = site_number

port_num = 8082
networks = []
stringbuild = ""

class BlockListNode:
    def __init__(self, site_num, machine_value, machine_num, buffer):
        self.site_num = site_num
        self.machine_value = machine_value
        self.machine_num = machine_num
        self.buffer = buffer
        self.next = None
        self.previous = None
    
    def add(self, new_node):
        if new_node.site_num > self.site_num:
            if self.next == None or new_node.site_num < self.next.site_num:
                new_node.next = self.next
                new_node.previous = self
                self.next = new_node
            else:
                self.next.add(new_node)
        else:
            if new_node.machine_value > self.machine_value:
                #print("new node's machine type is greater than our machine type, push it down the line")
                if self.next == None or new_node.machine_value < self.next.machine_value:
                    new_node.next = self.next
                    new_node.previous = self
                    self.next = new_node
                else:
                    self.next.add(new_node)
            elif new_node.machine_value == self.machine_value:
                #print("new node's machine type is the same as our machine type, find where it goes")
                if new_node.machine_num > self.machine_num:
                    #print("it goes forwards")
                    if self.next == None or new_node.machine_num < self.next.machine_num:
                        new_node.next = self.next
                        new_node.previous = self
                        self.next = new_node
                    else:
                        self.next.add(new_node)
                else:
                    #print("it goes backwards")
                    self.previous.next = new_node
                    new_node.previous = self.previous
                    new_node.next = self
                    self.previous = new_node
            else:
                #print("new node's machine type is less than our machine type, push it backwards")
                self.previous.next = new_node
                new_node.previous = self.previous
                new_node.next = self
                self.previous = new_node
        
    def print_buffer(self):
        global stringbuild
        stringbuild += self.buffer
        if self.next != None:
            self.next.print_buffer()
            

docker_compose_list = BlockListNode(-2, 100, -1, "")

class ConfigTreeNode:
    def __init__(self, name, machine_type, path, site, version):
        self.name = name
        self.machine_type = machine_type
        self.number = name[name.rfind("-")+1:]
        self.path = path
        self.site = site
        self.version = version
        # Initialize an empty array of children and networks for tree building later
        self.children = []
        self.networks = []
        # Shows whether this node has been exported to docker-compose.yml yet
        self.exported = False
        # Shows whether this node has a unique config set or software version
        self.override = False
    
    def add_child(self, child):
        self.children.append(child)
    
    def assign_networks(self):
        #print("In assign networks: "+self.site+" "+self.name+" "+self.machine_type+" "+self.number)
        if not any(network.name == "fleet_net" for network in networks):
            networks.append(Network("fleet_net","10.10.1.0/24"))
        if self.machine_type == "fleet_manager":
            self.networks.append(Network("fleet_net","10.10.1."+str(int(self.number)+4)))
            self.site_num = 0
        elif self.machine_type != "root":
            if not any(self.site in network.name for network in networks):
                count = 0
                for network in networks:
                    if "site" in network.name:
                        count += 1 
                networks.append(Network(self.site+"_site_net","192.168."+str(10+2*count)+".0/24", count+1))
                networks.append(Network(self.site+"_ess_net","192.168."+str(11+2*count)+".0/24", count+1))
            site_num = next((network for network in networks if self.site in network.name), None).site_number-1
            self.site_num = site_num + 1
            if self.machine_type == "site_controller":
                self.networks.append(Network("fleet_net","10.10.1."+str(int(site_num)+9)))
                self.networks.append(Network(self.site+"_site_net","192.168."+str(10+2*site_num)+"."+str(int(self.number)+4)))
            elif self.machine_type == "ess_controller":
                # add to site_net and ess_net
                self.networks.append(Network(self.site+"_site_net","192.168."+str(10+2*site_num)+"."+str(int(self.number)+9)))
                self.networks.append(Network(self.site+"_ess_net","192.168."+str(11+2*site_num)+"."+str(int(self.number)+9)))
            elif self.machine_type == "twins":
                # add to site_net and ess_net if not already there
                if not self.networks:
                    self.networks.append(Network(self.site+"_site_net","192.168."+str(10+2*site_num)+"."+str(int(self.number)+249)))
                    self.networks.append(Network(self.site+"_ess_net","192.168."+str(11+2*site_num)+"."+str(int(self.number)+4)))
            else:
                print("No machine type detected")
        for child in self.children:
            child.assign_networks()
    
    def print_tree(self, depth=0):
        strbuf = ""
        for i in range(depth):
            strbuf += "  "
        strbuf += self.site+" "+self.name+":"+self.version+"  - "+self.number
        print(strbuf)
        for child in self.children:
            child.print_tree(depth+1)
    
    def docker_compose_write(self, depth=0):
        # root is responsible for services and networks
        if self.name == "root":
            prefix = "version: '3'\n"
            prefix += "services:\n"
            docker_compose_list.add(BlockListNode(-1, 0, 0, prefix))
            for child in self.children:
                child.docker_compose_write(depth+1)
            suffix = "networks:\n"
            for network in networks:
                suffix += "  "+network.name+":\n"
                suffix += "    ipam:\n"
                suffix += "      driver: default\n"
                suffix += "      config:\n"
                suffix += "        - subnet: "+network.ip_addr+"\n\n"
            docker_compose_list.add(BlockListNode(255, 255, 255, suffix))
            return
        # If this Node has already been added to docker-compose.yml, don't add it
        if self.exported:
            return
        # Create docker-compose.yml block for this machine
        block_buff = "  "+self.site+"-"+self.name+":\n"
        # if version has letters in it, use a snapshot image, otherwise use a release image
        verletters = re.search('[a-zA-Z]', self.version)
        if verletters:
            block_buff += "    image: flexgen/"+self.machine_type+"-snapshot:"+self.version+"\n"
        else:
            block_buff += "    image: flexgen/"+self.machine_type+":"+self.version+"\n"
        # Set up host for this machine
        block_buff += "    container_name: "+self.site+"-"+self.name+"\n"
        block_buff += "    hostname: "+self.site+"-"+self.name+"\n"
        # Open ports for the UI
        if "twins" not in self.machine_type:
            global port_num
            block_buff += "    ports:\n"
            block_buff += "      - "+str(port_num)+":443\n"
            port_num += 1
        # Output network information for this machine
        block_buff += "    networks:\n"
        for network in self.networks:
            block_buff += "      "+network.name+":\n"
            block_buff += "        ipv4_address: "+network.ip_addr+"\n"
        block_buff += "    tty: true\n"
        block_buff += "    volumes:\n"
        # Output bind mount information for this machine
        if self.override:
            block_buff += "      - "+self.path+"/"+self.name+"/config:/home/staging\n\n"
        else:
            block_buff += "      - "+self.path+"/"+self.machine_type.replace("_","-")+"/config:/home/staging\n\n"
        # Encode machine type as a numerical value for ease of sorting
        machine_value = 3
        if "fleet" in self.machine_type:
            machine_value = 0
        elif "site" in self.machine_type:
            machine_value = 1
        elif "ess" in self.machine_type:
            machine_value = 2
        #print("Adding "+self.site+" "+self.machine_type+" "+self.number+" to docker-compose list")
        docker_compose_list.add(BlockListNode(self.site_num, machine_value, self.number, block_buff))
        # Mark self completed
        self.exported = True
        # Generate docker-compose for children
        for child in self.children:
            child.docker_compose_write(depth+1)



# Remove existing docker-compose.yml
if os.path.isfile("docker-compose.yml"):
    print ("detected docker-compose file; removing it.")
    os.remove("docker-compose.yml")

# Define lists for each level of machine type
fleet_managers = []
site_controllers = []
ess_controllers = []
twins = []
# Separate list for individual machines with config or version edits
override_nodes = []

# Walk through directory structure to build all nodes
cfg_dir = os.getcwd().replace("/bootstrap", "/config/")
for dirs in os.walk(cfg_dir):
    # Datamine the name, machine_type, path, and site of the node
    name = dirs[0][dirs[0].rfind('/')+1:]
    machine_type = name.replace("-","_")
    path = dirs[0][:dirs[0].rfind('/')]
    site = path[path.rfind('/')+1:]
    # If the directory isn't a machine on a site, continue the loop
    if ("fleet-manager" not in name) and ("site-controller" not in name) and ("ess-controller" not in name) and ("twins" not in name):
        continue
    # If the name of the machine has a digit in it, machine type needs the digits removed
    digit = re.search(r"\d", name)
    if digit:
        machine_type = name[:digit.start()-1].replace("-","_")
    
    nodes = []
    if machine_type == "fleet_manager":
        nodes = fleet_managers
        site = "data-center"
    elif machine_type == "site_controller":
        nodes = site_controllers
    elif machine_type == "ess_controller":
        nodes = ess_controllers
    elif machine_type == "twins":
        nodes = twins
    else:
        print("Unknown machine type detected")
    
    # Get list of all .yml files in directory
    for filename in os.listdir(dirs[0]):
        if not ".yml" in filename:
            continue
        # Append this node to the correct category of machine types
        with open(dirs[0]+"/"+filename, "r") as stream:
            try:
                info = yaml.safe_load(stream)
                if digit:
                    override_nodes.append(ConfigTreeNode(name, machine_type, path, site, info["version"]))
                    break
                else:
                    filenum = re.findall(r'\d+', filename)
                    nodes.append(ConfigTreeNode(name+"-"+"{:02d}".format(int(filenum[0])), machine_type, path, site, info["version"]))
            except yaml.YAMLError as exc:
                print(exc)

# override the nodes that have separate configurations specified
for node in override_nodes:
    node.override = True
    if node.machine_type == "fleet_manager":
        for fleet_manager in fleet_managers:
            if fleet_manager.name == node.name:
                fleet_managers.remove(fleet_manager)
        fleet_managers.append(node)
    elif node.machine_type == "site_controller":
        for site_controller in site_controllers:
            if site_controller.name == node.name and site_controller.site == node.site:
                site_controllers.remove(site_controller)
        site_controllers.append(node)
    elif node.machine_type == "ess_controller":
        for ess_controller in ess_controllers:
            if ess_controller.name == node.name and ess_controller.site == node.site:
                ess_controllers.remove(ess_controller)
        ess_controllers.append(node)
    elif node.machine_type == "twins":
        for twin in twins:
            if twin.name == node.name and twin.site == node.site:
                twins.remove(twin)
        twins.append(node)
    else:
        print("Unknown machine type detected")

# Build Tree and assign IP addresses
root = ConfigTreeNode("root", "root", cfg_dir, "data-center", "0.0")

for fleet_manager in fleet_managers:
    root.add_child(fleet_manager)

for site_controller in site_controllers:
    if fleet_managers:
        for fleet_manager in fleet_managers:
            fleet_manager.add_child(site_controller)
    else:
        root.add_child(site_controller)

for ess_controller in ess_controllers:
    if site_controllers:
        for site_controller in site_controllers:
            if site_controller.site == ess_controller.site:
                site_controller.add_child(ess_controller)
    elif fleet_managers:
        for fleet_manager in fleet_managers:
            fleet_manager.add_child(ess_controller)
    else:
        root.add_child(ess_controller)

for twin in twins:
    if ess_controllers:
        for ess_controller in ess_controllers:
            if ess_controller.site == twin.site:
                ess_controller.add_child(twin)
    if site_controllers:
        for site_controller in site_controllers:
            if site_controller.site == twin.site:
                site_controller.add_child(twin)
    if not ess_controllers and not site_controllers:
        root.add_child(twin)

#root.print_tree()
root.assign_networks()
root.docker_compose_write()
docker_compose_list.print_buffer()
file = open("docker-compose.yml", 'a')
file.write(stringbuild)
file.close()
