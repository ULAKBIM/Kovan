KVN_PREFIX=/usr/local/kovan

all:; @echo 'run "make install" or "make deinstall"' 


install:

	mkdir -p ${KVN_PREFIX}/bin
	mkdir -p ${KVN_PREFIX}/etc
	mkdir -p ${KVN_PREFIX}/state
	cp etc/snmpd.conf ${KVN_PREFIX}/router/etc/
	cp etc/snmpd.conf ${KVN_PREFIX}/blackhole/etc/
	cp etc/kovanTopology.conf  ${KVN_PREFIX}/etc/
	cp etc/MrtgPpsTemplate.txt  ${KVN_PREFIX}/monitor/etc/
	cp etc/kovanNagiosCheck  ${KVN_PREFIX}/monitor/usr/local/libexec/nagios/
	cp etc/syslog-ng.conf  ${KVN_PREFIX}/monitor/usr/local/etc
	cp etc/index.html  ${KVN_PREFIX}/monitor/usr/local/www/apache22/data
	install -m555 bin/kovan ${KVN_PREFIX}/bin/
	install -m555 bin/kovanMonitorServices ${KVN_PREFIX}/bin/
	install -m555 bin/kovanState ${KVN_PREFIX}/bin/
	install -m555 bin/kovanStats ${KVN_PREFIX}/bin/
	install -m555 bin/kovanNagios ${KVN_PREFIX}/bin/
	install -m555 bin/kovanQuaggaConf ${KVN_PREFIX}/bin/
	install -m555 bin/kovanApachePhp ${KVN_PREFIX}/bin/
	install -m555 bin/kovanNfsen ${KVN_PREFIX}/bin/
	install -m555 bin/kovanMrtg ${KVN_PREFIX}/bin/
	install -m555 bin/kovanManageLinks ${KVN_PREFIX}/bin/
	install -m555 bin/kovanrc /usr/local/etc/rc.d/kovan
	install -m555 etc/kovanDefault.conf ${KVN_PREFIX}/etc

deinstall:
	rm -f ${KVN_PREFIX}/bin/kovan
	rm -f ${KVN_PREFIX}/bin/kovanMonitorServices
	rm -f ${KVN_PREFIX}/bin/kovanState
	rm -f ${KVN_PREFIX}/bin/kovanStats
	rm -f ${KVN_PREFIX}/bin/kovanNagios
	rm -f ${KVN_PREFIX}/bin/kovanQuaggaConf
	rm -f ${KVN_PREFIX}/bin/kovanApachePhp
	rm -f ${KVN_PREFIX}/bin/kovanNfsen
	rm -f ${KVN_PREFIX}/bin/kovanMrtg
	rm -f ${KVN_PREFIX}/bin/kovanManageLinks
	rm -f /usr/local/etc/rc.d/kovan
	rm -f ${KVN_PREFIX}/etc/kovanTopology.conf
	rm -f ${KVN_PREFIX}/monitor/usr/local/libexec/nagios/kovanNagiosCheck
	rm -f ${KVN_PREFIX}/monitor/etc/MrtgPpsTemplate.txt
	rm -f ${KVN_PREFIX}/monitor/usr/local/etc/syslog-ng.conf
	rm -f ${KVN_PREFIX}/etc/kovanDefault.conf
