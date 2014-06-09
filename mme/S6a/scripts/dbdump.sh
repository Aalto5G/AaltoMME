#!/bin/bash
#

FILE=hss_lte_db.sql

mysqldump hss_lte_db --single-transaction -u hss -p -d -B --add-drop-table --add-drop-database >$FILE
echo "# DB access rights" >>$FILE
echo "grant delete,insert,select,update on hss_lte_db.* to hss@localhost identified by 'hss';" >>$FILE

FILE=userdata.sql
mysqldump hss_lte_db --single-transaction -u hss -p -t -B  >$FILE




