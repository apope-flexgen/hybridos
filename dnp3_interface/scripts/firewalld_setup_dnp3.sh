sudo firewall-cmd --zone=public --add-service=http
sudo firewall-cmd --zone=public --add-service=https
sudo firewall-cmd --zone=public --add-port=502/tcp
sudo firewall-cmd --zone=public --add-port=20000/tcp
sudo firewall-cmd --zone=public --add-port=20001/tcp
sudo firewall-cmd --zone=public --add-port=20002/tcp
#sudo firewall-cmd --zone=public --add-port=3001/tcp
#sudo firewall-cmd --zone=public --add-port=3002/tcp
