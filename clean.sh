#!/bin/bash

sudo git clean -xdf

cd ./package_utility

cd ../dbi/ && sudo git clean -xdf
cd ../dnp3_interface/ && sudo git clean -xdf
cd ../ess_controller/ && sudo git clean -xdf
cd ../events/ && sudo git clean -xdf
cd ../fims/ && sudo git clean -xdf
cd ../metrics/ && sudo git clean -xdf
cd ../modbus_interface/ && sudo git clean -xdf
