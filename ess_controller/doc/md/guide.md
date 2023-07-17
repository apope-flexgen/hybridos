# Getting Started

## Building the ESS Controller
Refer to [verson.md](version.md) for a list of all git repositories and their current branch used for a particular version of the ESS Controller.  

### Configuration
The RPMs and configuration files used for the ESS Controller are located in the `config_ess` repository. To access these configs:

Clone the config_ess repository   
`$ git clone git@github.com:flexgen-power/config_ess.git`

Switch to the config_ess repository  
`$ cd config_ess`

Run the following to install RPMs and configs on the target ess box   
`$ ./deploy_and_install_on_site.sh [username] [ip_address]`  

### Development
Clone the ESS Controller repository  
`$ git clone git@github.com:flexgen-power/ess_controller.git`

Switch to the ESS Controller repository  
`$ cd ess_controller`

Build the ESS Controller  
`$ ./package_utility/build.sh`  

### Production

----

## Running the ESS Controller
To run the modbus client(s), modbus server(s), events module, databases, the ESS Controller, web server/ui, and other relevant processes:

Switch to the config_ess repository  
`$ cd config_ess`

Go to the scripts directory  
`$ cd install/scripts`

Run the firewall script if it hasn't been run already 
`$ sudo ./firewalld_setup.sh`

Run the start script  
`$ ./ess_run.sh`

Run the stop script to shutdown the ESS Controller and other running processes  
`$ ./ess_stop.sh`

Note: To check the ESS Controller web interface, go to https://[ip_address_of_machine_running_ess_controller]  
From there, there should be a login page where you can enter your credentials
