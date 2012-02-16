#!/bin/sh

ONEDAYBEFORE=`date -v-1d "+%Y-%m-%d"`
DEST=/usr/local/www/stats

# usage:    kovan_graph --outputfile NAME --xaxis TITLE --yaxis TITLE --query QUERY --header HEADER

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/protocol-daily$ONEDAYBEFORE.png" --xaxis "Protocol" --yaxis "Attack" --header "$ONEDAYBEFORE" --query "SELECT proto, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(DATE_SUB(CURDATE(),INTERVAL 1 DAY)) and srcip<>\"2001:a98:14:5::2\" GROUP BY proto ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/srcip-daily$ONEDAYBEFORE.png" --xaxis "Source IP" --yaxis "Attack" --header "$ONEDAYBEFORE" --query "SELECT srcip, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(DATE_SUB(CURDATE(),INTERVAL 1 DAY)) and srcip<>\"2001:a98:14:5::2\" GROUP BY srcip ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/destip-daily$ONEDAYBEFORE.png" --xaxis "Destination IP" --yaxis "Attack" --header "$ONEDAYBEFORE" --query "SELECT destip, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(DATE_SUB(CURDATE(),INTERVAL 1 DAY)) and srcip<>\"2001:a98:14:5::2\" GROUP BY destip ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/destport-daily$ONEDAYBEFORE.png" --xaxis "Destination PORT" --yaxis "Attack" --header "$ONEDAYBEFORE" --query "SELECT destport, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(DATE_SUB(CURDATE(),INTERVAL 1 DAY)) and destport<>-1 and srcip<>\"2001:a98:14:5::2\" GROUP BY destport ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/service-daily$ONEDAYBEFORE.png" --xaxis "Destination Service" --yaxis "Attack" --header "$ONEDAYBEFORE" --query "SELECT service, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(DATE_SUB(CURDATE(),INTERVAL 1 DAY)) and service <> 'blackhole' and srcip<>\"2001:a98:14:5::2\"  GROUP BY service ORDER BY counter DESC LIMIT 10"

