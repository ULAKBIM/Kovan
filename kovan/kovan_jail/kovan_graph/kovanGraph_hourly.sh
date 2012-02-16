#!/bin/sh

TODAY=`date "+%Y_%m_%d"`
TODAYv2=`date "+%Y-%m-%d"`
THISHOUR=`date +%H`
ONEHOURBEFORE=`date -v-1H +%H`

DEST=/usr/local/www/stats


/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/protocol-hourly$ONEHOURBEFORE-$TODAY.png" --xaxis "Protocol" --yaxis "Attack" --header "$TODAYv2[$ONEHOURBEFORE-$THISHOUR]" --query "SELECT proto, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(CURDATE()) and HOUR(FROM_UNIXTIME(logtime))=HOUR(CURTIME())-1 and srcip<>\"2001:a98:14:5::2\" GROUP BY proto ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/srcip-hourly$ONEHOURBEFORE-$TODAY.png" --xaxis "Source IP" --yaxis "Attack" --header "$TODAYv2[$ONEHOURBEFORE-$THISHOUR]" --query "SELECT srcip, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(CURDATE()) and HOUR(FROM_UNIXTIME(logtime))=HOUR(CURTIME())-1 and srcip<>\"2001:a98:14:5::2\" GROUP BY srcip ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/destip-hourly$ONEHOURBEFORE-$TODAY.png" --xaxis "Destination IP" --yaxis "Attack" --header "$TODAYv2[$ONEHOURBEFORE-$THISHOUR]" --query "SELECT destip, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(CURDATE()) and HOUR(FROM_UNIXTIME(logtime))=HOUR(CURTIME())-1 and srcip<>\"2001:a98:14:5::2\" GROUP BY destip ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/destport-hourly$ONEHOURBEFORE-$TODAY.png" --xaxis "Destination PORT" --yaxis "Attack" --header "$TODAYv2[$ONEHOURBEFORE-$THISHOUR]" --query "SELECT destport, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(CURDATE()) and HOUR(FROM_UNIXTIME(logtime))=HOUR(CURTIME())-1 and destport<>-1 and srcip<>\"2001:a98:14:5::2\" GROUP BY destport ORDER BY counter DESC LIMIT 10"

/usr/local/kovan/bin/kovan_graph --outputfile "$DEST/service-hourly$ONEHOURBEFORE-$TODAY.png" --xaxis "Service" --yaxis "Attack" --header "$TODAYv2[$ONEHOURBEFORE-$THISHOUR]" --query "SELECT service, count(*) as counter FROM logs where DATE(FROM_UNIXTIME(logtime))=DATE(CURDATE()) and HOUR(FROM_UNIXTIME(logtime))=HOUR(CURTIME())-1 and service <> 'blackhole' and srcip<>\"2001:a98:14:5::2\" GROUP BY service ORDER BY counter DESC LIMIT 10"

