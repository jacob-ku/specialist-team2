#!/bin/bash

if [ $# -ne 3 ]; then
    echo "This script need 3 parameter"
    echo "Usage : ./zzuf_test.sh [Start Seed] [End Seed] [Input File]"
    exit 1
fi

it_start=$1
it_end=$2
input=$3
echo "iteration : [${it_start} - ${it_end}]"
echo "input file : ${input}"

TestCase_DIR=./TCs
input_backup=${input}.ori

if [ ! -d $TestCase_DIR ]; then
    mkdir $TestCase_DIR
fi

cp ${input} ${input_backup}

for ((i = ${it_start}; i < ${it_end}; i++));
do
    tc_filename=${i}_input
    zzuf -s$i -r.1:1 < ${input} > ${TestCase_DIR}/${tc_filename}
    cp ${TestCase_DIR}/${tc_filename} ${input}
    
    result=`./LgFaceRecDemoTCP_Jetson_NanoV2 5000 2<&1 > /dev/null`
    ret=$?
    echo "[${i}] ret : ${ret}"
    if [ ${ret} -eq 139 ]; then
        echo "${i} : Segmentation Fault!!!!!!!!!!!!"
	exit 1
    fi

    cp ${input_backup} ${input}
done
