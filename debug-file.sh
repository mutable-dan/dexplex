ps -ax |grep dexplex|grep -v vi|head -n 1
pid=$(ps -ax |grep dexplex|grep -v vi|head -n 1|tr -s ' '|cut -d ' ' -f2)
echo 'pid:' $pid
# SIGUSR1
kill -10 $pid
