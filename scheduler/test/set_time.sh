if [ $# != 3 ]; then
    echo -e "invalid arguments. expecting \$1 = Hours, \$2 = Minutes, \$3 = Seconds"
    exit 1
fi
sudo date --set="$1:$2:$3" +"%T"