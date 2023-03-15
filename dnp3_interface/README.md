# dnp3_interface for fleetmanager branch
1. currently build and clean functions are not implemented
2. To get the RPM, create folder build/release in repo
3. copy binaries dnp3_master, dnp3_outstation to build/release created in step 3
4. copy libdmap.so file to build/release
5. copy files generated from building opendnp3 repo to build/release folder
6. copy dnp3_master.service and dnp3_outstation.service to build/release
7. This rpm requires a total of 9 files. Make sure there are 9 files copied to build/release
8. to generate RPM call ./package_utility/package.sh
9. This should create rpm file in ~/rpmbuild/RPMS/x86_64
10. To install rpm use this command
	sudo yum localinstall -y --disablerepo=* --nogpgcheck <rpmfile>


If any issues fix it.

