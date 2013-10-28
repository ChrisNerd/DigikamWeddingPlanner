#!/bin/bash
# createAssociationMatrix.sh
# Author: Chris Shannon <chris.shannon@gmail.com>
# This script is released under a CC-GNU GPL License
#
# usage ./createAssociationMatrix.sh > mat.txt && g++ partition.cpp && ./a.out < mat.txt 
#
# A bash script to analyze the content of Digikam's SQLite Database and output the number of photos in common for each pair of people from a specified set.
# Reads a file called guestlist.txt, which is a list of names, one per line, that may or may not be in the digikam database.  For the names in the database,
# all the pairs are considered, and for each pair the number of photos (and videos) that have both members in it is returned.
# For the names not in the database, the names are just echoed back with an id.  They must be giving associations with at the next step, extraAssociations.txt

# The file extraAssociations.txt should include a line for each name in guestlist.txt that doesn't have an entry in the digikam database.  That line must be
# a comma seperated list of friends, e.g. Barry Ing,Roberta Jone,Gerald Ing,Ted Ing,Nova Broens     This is the friend list of Barry Ing.  The friends, e.g. Roberta,
# Gerald, etc. may or may not exist in the database.

which kde4-config >/dev/null 2>&1 # Testing whether kde4-config is available
if [ $? == 0 ]; then
	DIGIKAMRC=`kde4-config --localprefix`/share/config/digikamrc
else
	which kde-config >/dev/null 2>&1 # Testing whether kde-config is available
	if [ $? == 0 ]; then
		DIGIKAMRC=`kde-config --localprefix`/share/config/digikamrc
	else
		DIGIKAMRC=$HOME/.kde/share/config/digikamrc # Assuming standard path
	fi
fi
if [ ! -f "$DIGIKAMRC" ]; then
	echo "Cannot find Digikam config file (digikamrc)"
	exit 1
fi

DBTYPE=`grep Database\ Type $DIGIKAMRC | cut -d '=' -f 2` # Lookup Database backend type
if [ -z $DBTYPE ]; then
#	echo "Backend is SQLite"
	DIGIKAMDB=`grep Database\ File\ Path= $DIGIKAMRC | cut -d '=' -f 2`/digikam4.db # SQLite DB file
	DIGIKAMDB=`echo $DIGIKAMDB | sed 's/\/\//\//'`
	SQLCOMMAND="sqlite3 -separator "," $DIGIKAMDB" # set comma as separator to create csv files
	DBTYPE="SQLITE"
elif [ $DBTYPE == "QSQLITE" ]; then
#	echo "Backend is SQLite"
	DIGIKAMDB=`grep Database\ Name= $DIGIKAMRC | cut -d '=' -f 2`/digikam4.db # SQLite DB file
	DIGIKAMDB=`echo $DIGIKAMDB | sed 's/\/\//\//'`
	SQLCOMMAND="sqlite3 -separator "," $DIGIKAMDB" # set comma as separator to create csv files
elif [ $DBTYPE == "QMYSQL" ]; then
	INTERNALDBSERVER=`grep Internal\ Database\ Server $DIGIKAMRC | cut -d '=' -f 2` # check usage of MySQL
	if [ ! $INTERNALDBSERVER == "false" ]; then
		echo "Internal MYSQL Server is not supported at the moment"
		exit 1
	fi
	DBHOSTNAME=`grep Database\ Hostname $DIGIKAMRC | cut -d '=' -f 2` # Lookup hostname 
	if [ ! -z $DBHOSTNAME ] ; then
		DBHOSTNAME="-h $DBHOSTNAME"
	fi
	DBPORT=`grep Database\ Port $DIGIKAMRC | cut -d '=' -f 2` # Lookup port number
	if [ ! -z $DBPORT ] ; then
		DBPORT="-P $DBPORT"
	fi
	DIGIKAMDB=`grep Database\ Name= $DIGIKAMRC | cut -d '=' -f 2` # Lookup DB name
	DBUID=`grep Database\ Username $DIGIKAMRC | cut -d '=' -f 2` # Lookup username
	if [ ! -z $DBUID ] ; then
		DBUID="-u $DBUID"
	fi
	DBPWD=`grep Database\ Password $DIGIKAMRC | cut -d '=' -f 2` # Lookup password
	if [ ! -z $DBPWD ] ; then
		DBPWD="-p$DBPWD"
	fi
	DBSOCKET=`grep "^Database Connectoptions=UNIX_SOCKET=" $DIGIKAMRC | cut -d '=' -f 3` # Get the socket 
	if [ ! -z $DBSOCKET ] ; then
		DBSOCKET="-S $DBSOCKET"
	fi
	SQLCOMMAND="mysql $DBUID $DBPWD $DBHOSTNAME $DBPORT $DBSOCKET -B -N $DIGIKAMDB -e "
fi

GUESTCOUNT="0";
while read NAME1; do
ID1=`$SQLCOMMAND "select id from Tags where name is '$NAME1'"`;
echo $ID1 $NAME1 $GUESTCOUNT
NAMES[$GUESTCOUNT]=$NAME1
IDS[$GUESTCOUNT]=$ID1
GUESTCOUNT=`expr $GUESTCOUNT + 1`;
done < guestlist.txt
echo "AndNowForTheAssociations"
for id1 in "${IDS[@]}"
do
 if [[ -n $id1 ]]; then
  for id2 in "${IDS[@]}"
  do
   if [[ -n $id2 ]]; then
    if [[ "$id1" -eq "$id2" ]]; then
     COUNTCOMMON=`$SQLCOMMAND "select count(*) from ImageTags where tagid='$id1'"`
    else
     COUNTCOMMON=`$SQLCOMMAND "select count(*) from (select imageid, count(*) from ImageTags where tagid='$id1' or tagid='$id2' group by imageid having count(*)>1)"`
    fi
    echo $id1 $id2 $COUNTCOMMON
   fi
  done
 fi
done
# fudges to ensure that me and Donna stay together
sed -i -e 's/11 12 .*/11 12 11300/g' -e 's/12 11 .*/12 11 11300/g' -e 's/587 588 .*/587 588 -100/g' -e 's/113 153 .*/113 153 -100/g' -e 's/153 113 .*/153 113 -100/g' -e 's/14 30 .*/14 30 2000/g' -e 's/30 14 .*/30 14 2000/g' mat.txt
