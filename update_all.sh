#!bin/sh
#example ../src/engine/restart_cnoda_ssh.sh root 192.168.0.200 /www/pages/necron/Cnoda
user=$1
ip=$2
path=$3
binPath=$4

#ssh $user'@'$ip 'killall safe_logger'
#ssh $user'@'$ip 'killall watchdog_stub'
#ssh $user'@'$ip 'sv stop /var/run/sv//www/pages/necron/Cnoda/Cnoda.conf=wwwpagesnecronCnodaCnoda.json'
#ssh $user'@'$ip 'sv stop /var/run/sv//www/pages/necron/Cnoda/snmpagnt.conf=varrunsubagent.json'
#echo "sleep 1sec"
#ssh $user'@'$ip 'sleep 1'
#ssh $user'@'$ip 'killall -9 Cnoda'
#ssh $user'@'$ip '/etc/init.d/slogger.sh restart'
#ssh $user'@'$ip 'killall -9 watchdog_stub'
#echo "sleep 1sec"
#ssh $user'@'$ip 'sleep 1'
#ssh $user'@'$ip 'sv stop /www/pages/necron/Cnoda/safe_logger /www/pages/necron/Cnoda/Cnoda.json'

echo Update all


	ssh $user'@'$ip 'sh /www/pages/scripts/stop_cnoda.sh'
	sleep 1
	ssh $user'@'$ip 'sh /www/pages/scripts/stop_cnoda.sh'
	sleep 1
	echo Update Cnoda
	scp $binPath/Cnoda $user'@'$ip':'$path
	echo scp $binPath/Cnoda $user'@'$ip':'$path
	scp $binPath/svc $user'@'$ip':'$path
	echo scp $binPath/necron $user'@'$ip':'$path
	scp $binPath/necron $user'@'$ip':'$path
	echo scp $binPath/ssdpd $user'@'$ip':'$path
	scp $binPath/ssdpd $user'@'$ip':'$path
	echo scp $binPath/watchdog_stub $user'@'$ip':'$path
	scp $binPath/watchdog_stub $user'@'$ip':'$path
	echo scp $binPath/libcnoda* $user'@'$ip':'/usr/lib
	scp $binPath/libcnoda* $user'@'$ip':'/usr/lib

	sleep 1


#ssh $user'@'$ip 'sv start /var/run/sv//www/pages/necron/Cnoda/Cnoda.conf=wwwpagesnecronCnodaCnoda.json'
#ssh $user'@'$ip 'sv start /var/run/sv//www/pages/necron/Cnoda/snmpagnt.conf=varrunsubagent.json'


#ssh $user'@'$ip 'cp /www/pages/necron/Cnoda/Cnoda /www/pages/necron/Cnoda/watchdog_stub'
#ssh $user'@'$ip 'nohup /www/pages/necron/Cnoda/watchdog_stub > /var/run/foo.out 2> /var/run/foo.err < /dev/null &'
echo 'Ok'
#ssh $user'@'$ip 'sv start /www/pages/necron/Cnoda/safe_logger /www/pages/necron/Cnoda/Cnoda.json'
