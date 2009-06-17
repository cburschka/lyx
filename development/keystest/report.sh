#!/bin/bash
LT=development/keystest

# echo 'grep "#1 " $LT/out/GDB | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq' >> report.sh

UNIQUE_LINE=1

while [ ! -e $LT/out/GDB ]
do
  cd ..
  if [ `pwd` = '/' ]
  then
     exit
  fi
done

strings $LT/out/*GDB > $LT/out/GDBs 
grep "#$UNIQUE_LINE " $LT/out/GDBs > $LT/out/list
#cat $LT/out/list | grep -o ' in [[:alnum:]:]* ' | sort | uniq| tee $LT/out/listuniq
#cat $LT/out/list | grep -o ' in [[:alnum:]:]* ' | sort | uniq| tee $LT/out/listuniq
cat $LT/out/list | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq | tee $LT/out/listuniq

NUM_REPORTS=`wc -l < $LT/out/list`
echo NUM_REPORTS $NUM_REPORTS

echo > $LT/out/overview

cat $LT/out/listuniq | while read l 
do
  #name=`echo $l | sed s/in// | sed 's/ //g'`
  grep "$l" -B $UNIQUE_LINE -A 100 $LT/out/GDBs | head -n 100 | grep '#[0-9]' >$LT/out/tmp
  name=`cat $LT/out/tmp | ( grep -o ' in lyx::[[:alnum:]:]*' || cat $LT/out/tmp | grep -o ' [ai][nt] [[:alnum:]:]*' ) | head -n1 | sed s/in// | sed 's/ //g'`
  echo NAME: $name
  echo L: $l
 #out/list | grep -o ' in [[:alnum:]:]* ' | sort | uniq| tee out/listuniq
  (
  cat stock_text
  uname -r ; cat /etc/lsb-release| grep -i DISTRIB_DESCRIPTION
  (cd src && svn info | grep Rev:)
  echo
  NUM_OCCURANCES=`grep "$l" $LT/out/list | wc -l`
  echo number_of_occurances: $NUM_OCCURANCES/$NUM_REPORTS
  echo -e $NUM_OCCURANCES "\t" $name >> $LT/out/overview
  #echo grep "$l" $LT/out/list 
  #grep $l $LT/out/GDB
  echo '{{{'
  grep "$l" -B $UNIQUE_LINE -A 100 $LT/out/GDBs | head -n 100 | grep '#[0-9]'
  echo '}}}'
  ) | tee $LT/out/report_$name
done   

cat $LT/out/overview | sort -rn | tee LT/out/overview_sort
#cp $LT/out/r* $LT/share/
#cp $LT/out/o* $LT/share/
