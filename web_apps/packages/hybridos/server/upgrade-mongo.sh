# This is a temporary script to upgrade mongo from 3.4 to 6.0 in the VM env.
# Usage: bash upgrade-mongo.sh

sudo mongod --shutdown --config /etc/mongod.conf

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "3.4" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-3.6]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/3.6/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-3.6.asc' | sudo tee /etc/yum.repos.d/mongodb-org-3.6.repo
sudo yum install -y mongodb-org-3.6.23 mongodb-org-server-3.6.23 mongodb-org-shell-3.6.23 mongodb-org-mongos-3.6.23 mongodb-org-tools-3.6.23

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "3.6" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-4.0]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/4.0/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-4.0.asc' | sudo tee /etc/yum.repos.d/mongodb-org-4.0.repo
sudo yum install -y mongodb-org-4.0.27 mongodb-org-server-4.0.27 mongodb-org-shell-4.0.27 mongodb-org-mongos-4.0.27 mongodb-org-tools-4.0.27

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "4.0" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-4.2]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/4.2/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-4.2.asc' | sudo tee /etc/yum.repos.d/mongodb-org-4.2.repo
sudo yum install -y mongodb-org-4.2.18 mongodb-org-server-4.2.18 mongodb-org-shell-4.2.18 mongodb-org-mongos-4.2.18 mongodb-org-tools-4.2.18

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "4.2" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-4.4]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/4.4/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-4.4.asc' | sudo tee /etc/yum.repos.d/mongodb-org-4.4.repo
sudo yum install -y mongodb-org-4.4.17 mongodb-org-server-4.4.17 mongodb-org-shell-4.4.17 mongodb-org-mongos-4.4.17 mongodb-org-tools-4.4.17

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "4.4" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-5.0]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/5.0/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-5.0.asc' | sudo tee /etc/yum.repos.d/mongodb-org-5.0.repo
sudo yum install -y mongodb-org-5.0.13 mongodb-org-database-5.0.13 mongodb-org-server-5.0.13 mongodb-org-shell-5.0.13 mongodb-org-mongos-5.0.13 mongodb-org-tools-5.0.13

sudo mongod --config /etc/mongod.conf
mongo --eval 'db.adminCommand( { setFeatureCompatibilityVersion: "5.0" } )'
sudo mongod --shutdown --config /etc/mongod.conf
echo $'[mongodb-org-6.0]\nname=MongoDB Repository\nbaseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/6.0/x86_64/\ngpgcheck=1\nenabled=1\ngpgkey=https://www.mongodb.org/static/pgp/server-6.0.asc' | sudo tee /etc/yum.repos.d/mongodb-org-6.0.repo
sudo yum install -y mongodb-org-6.0.2 mongodb-org-database-6.0.2 mongodb-org-server-6.0.2 mongodb-mongosh-6.0.2 mongodb-org-mongos-6.0.2 mongodb-org-tools-6.0.2

sudo mongod --config /etc/mongod.conf
