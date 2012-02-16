KVN_PREFIX=/usr/local/kovan
KVN_BLACKHOLE_PREFIX=${KVN_PREFIX}/blackhole/usr/local/kovan
KVN_MONITOR_PREFIX=${KVN_PREFIX}/monitor/usr/local/kovan
KVN_NODE_PREFIX=${KVN_PREFIX}/node/usr/local/kovan

all: hole generic_server dns smtp http ftp example

generic_server:
	cd ./shared; make generic_server

dns: generic_server
	cd ./kovan_dns; make kovan_dns

smtp: generic_server
	cd ./kovan_smtp; make kovan_smtp

http: generic_server
	cd ./kovan_http; make kovan_http

ftp: generic_server
	cd ./kovan_ftp; make kovan_ftp

example: generic_server
	cd ./examples; make all

hole: 
	cd ./kovan_dump; make kovan_dump 

install:

	mkdir -p ${KVN_NODE_PREFIX}/bin
	mkdir -p ${KVN_BLACKHOLE_PREFIX}/bin
	mkdir -p ${KVN_MONITOR_PREFIX}/bin
	mkdir -p ${KVN_MONITOR_PREFIX}/etc
	mkdir -p ${KVN_PREFIX}/monitor/usr/local/www/stats
	install -m 0755 kovan_http/kovan_http ${KVN_NODE_PREFIX}/bin
	install -m 0755 kovan_ftp/kovan_ftp ${KVN_NODE_PREFIX}/bin
	install -m 0755 kovan_smtp/kovan_smtp ${KVN_NODE_PREFIX}/bin
	install -m 0755 kovan_dns/kovan_dns ${KVN_NODE_PREFIX}/bin
	install -m 0755 kovan_dump/kovan_dump ${KVN_BLACKHOLE_PREFIX}/bin
	
	install -m 0755 kovan_graph/kovan_graph ${KVN_MONITOR_PREFIX}/bin
	install -m 0755 kovan_graph/kovanGraph_daily.sh ${KVN_MONITOR_PREFIX}/bin
	install -m 0755 kovan_graph/kovanGraph_hourly.sh ${KVN_MONITOR_PREFIX}/bin
	install -m 0755 kovan_graph/droidsans.ttf ${KVN_MONITOR_PREFIX}/etc
	install -m 0755 kovan_graph/stats.php ${KVN_PREFIX}/monitor/usr/local/www/stats/index.php

	mkdir -p ${KVN_NODE_PREFIX}/etc
	mkdir -p ${KVN_NODE_PREFIX}/etc/dns
	mkdir -p ${KVN_NODE_PREFIX}/etc/smtp
	mkdir -p ${KVN_NODE_PREFIX}/etc/www

	cp kovan_dns/named.conf ${KVN_NODE_PREFIX}/etc/dns
	cp kovan_dns/ipv6go.ipv6.net.tr ${KVN_NODE_PREFIX}/etc/dns

	cp kovan_smtp/ipv6go_mail.conf ${KVN_NODE_PREFIX}/etc/smtp

deinstall:
	rm ${KVN_NODE_PREFIX}/bin/kovan_http
	rm ${KVN_NODE_PREFIX}/bin/kovan_ftp
	rm ${KVN_NODE_PREFIX}/bin/kovan_smtp
	rm ${KVN_NODE_PREFIX}/bin/kovan_dns
	rm ${KVN_BLACKHOLE_PREFIX}/bin/kovan_dump
	rm -r ${KVN_NODE_PREFIX}/etc/dns
	rm -r ${KVN_NODE_PREFIX}/etc/smtp
	rm -r ${KVN_NODE_PREFIX}/etc/www
	
	rm ${KVN_MONITOR_PREFIX}/bin/kovan_graph
	rm ${KVN_MONITOR_PREFIX}/bin/kovanGraph_daily.sh
	rm ${KVN_MONITOR_PREFIX}/bin/kovanGraph_hourly.sh
	rm ${KVN_MONITOR_PREFIX}/etc/droidsans.ttf
	rm ${KVN_PREFIX}/monitor/usr/local/www/stats/index.php

clean:
	cd ./shared; make clean
	cd ./kovan_dns; make clean
	cd ./kovan_smtp; make clean
	cd ./kovan_http; make clean
	cd ./kovan_ftp; make clean
	cd ./examples; make clean
	cd ./kovan_dump; make clean

