#!/bin/bash

config_dir=../config/tx10/cops
go run ./src/cops.go ./src/statistician.go ./src/c2c.go ./src/negotiator.go ./src/doctor.go $config_dir