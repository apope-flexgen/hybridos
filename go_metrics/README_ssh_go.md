#OK go does not like our ssh setup or, at least, it may not like it.
#possibly due to /tmp/ssh being mounted as read only under docker
#we need to run ssh-agent and add our private key to it.
#all this fails with /tmp/ssh because its has the wrong permissions.


#first start ssh-agent
#then see who's home.

eval `ssh-agent -s`
ssh-add -l


ssh-add /tmp/.ssh/id_rsa
#this may fail due to permissions

#if you have to modify the perms on  /tmp/.ssh you will have to make a copy to root
 
mkdir -p /root
cp -a  /tmp/.ssh /root
chmod 0600 /root/.ssh/id_rsa
ssh-add /root/.ssh/id_rsa

# known hosts may also screw you up.
rm /root/.ssh/known_hosts

# then you can try to build the package
go mod tidy

# base dir git/go_metrics 

go build -o $build/go_metrics ./src

