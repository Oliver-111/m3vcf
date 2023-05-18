#!/bin/bash

pid=$1          #获取进程pid
#echo $pid
interval=$2     #设置采集间隔
filename=$3   #'proc_mem_record.txt'
VmRSS='0'
cpu='0.0'
cVmRss='0'
ccpu='0.0'
while [ -f "/proc/$pid/status" ]
do
	date +"%y-%m-%d %H:%M:%S"|tr '\n' ' ' >> $filename
	cVmRss=`cat /proc/$pid/status|grep -e VmRSS|awk '{print $2}'`
	echo -e "VmRSS\t\t ${cVmRss}\c" >> $filename #获取内存占用
	if [ $VmRSS -lt $cVmRss ]
	then
		VmRSS=$cVmRss
		#echo "vmrss=$VmRSS"
	fi
	
	ccpu=`top -b -n 1 -p $pid|sed '1,7d'|awk '{print $9}'`
	echo -e "\t%cpu\t\t ${ccpu}" >> $filename #获取cpu占用
	if [ `echo "$cpu < $ccpu" |bc` -eq 1 ]
	then
		cpu=$ccpu
		#echo "cpu=$cpu"
	fi
	sleep $interval
done

echo "######################################################################################" >> $filename
echo "######################################################################################" >> $filename
echo "max VmRSS: ${VmRSS} kb                    max %cpu: $cpu     " >> $filename
echo "######################################################################################" >> $filename
echo "######################################################################################" >> $filename

