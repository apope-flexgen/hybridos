# Lists the open file descriptors currently held by the DTS process

pid=$(pgrep dts)
echo The process pid is $pid
echo The number of open files is $(lsof -p $pid | wc -l)
lsof -p $pid