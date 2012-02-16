#!/usr/local/bin/bash  

set -e
 
JAILS=( monitor router node blackhole )

install_main() {

    install_jails
    # assign directory path of kovan according to KVN_PREFIX for files which have PATH variable in it.
    KVN_PREFIX2=`echo $KVN_PREFIX | sed -e 's/\//\\\\\//g'`
    #kovanrc
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/bin/kovanrc > kovan_host/bin/kovanrc.yedek
    mv kovan_host/bin/kovanrc.yedek kovan_host/bin/kovanrc
    #kovanDefault.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanDefault.conf > kovan_host/etc/kovanDefault.conf.yedek
    mv kovan_host/etc/kovanDefault.conf.yedek kovan_host/etc/kovanDefault.conf
    #kovanDefault.cfg
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanDefault.cfg > kovan_host/etc/kovanDefault.cfg.yedek
    mv kovan_host/etc/kovanDefault.cfg.yedek kovan_host/etc/kovanDefault.cfg
    #kovanTest_128nodes.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_128nodes.conf > kovan_host/etc/kovanTest_128nodes.conf.yedek
    mv kovan_host/etc/kovanTest_128nodes.conf.yedek kovan_host/etc/kovanTest_128nodes.conf
    #kovanTest_16nodes.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_16nodes.conf > kovan_host/etc/kovanTest_16nodes.conf.yedek
    mv kovan_host/etc/kovanTest_16nodes.conf.yedek kovan_host/etc/kovanTest_16nodes.conf
    #kovanTest_256nodes.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_256nodes.conf > kovan_host/etc/kovanTest_256nodes.conf.yedek
    mv kovan_host/etc/kovanTest_256nodes.conf.yedek kovan_host/etc/kovanTest_256nodes.conf
    #kovanTest_32nodes.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_32nodes.conf > kovan_host/etc/kovanTest_32nodes.conf.yedek
    mv kovan_host/etc/kovanTest_32nodes.conf.yedek kovan_host/etc/kovanTest_32nodes.conf
    #kovanTest_512nodes.conf 
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_512nodes.conf > kovan_host/etc/kovanTest_512nodes.conf.yedek
    mv kovan_host/etc/kovanTest_512nodes.conf.yedek kovan_host/etc/kovanTest_512nodes.conf
    #kovanTest_64nodes.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTest_64nodes.conf > kovan_host/etc/kovanTest_64nodes.conf.yedek
    mv kovan_host/etc/kovanTest_64nodes.conf.yedek kovan_host/etc/kovanTest_64nodes.conf
    #kovanTopology.conf
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTopology.conf > kovan_host/etc/kovanTopology.conf.yedek
    mv kovan_host/etc/kovanTopology.conf.yedek kovan_host/etc/kovanTopology.conf
    #kovanTopology.conf-test
    sed -e "s/PATH/$KVN_PREFIX2/g" kovan_host/etc/kovanTopology.conf-test > kovan_host/etc/kovanTopology.conf-test.yedek
    mv kovan_host/etc/kovanTopology.conf-test.yedek kovan_host/etc/kovanTopology.conf-test
    cd kovan_host 
    make install KVN_PREFIX=$KVN_PREFIX
    cd ..
    cd kovan_jail
    make 
    make install KVN_PREFIX=$KVN_PREFIX
}

install_usage() {
    echo "usage: $0 PREFIX IP INTERFACE"
    echo " -PREFIX: Target Path that Kovan will be installed (i.e /usr/local/kovan)."
    echo " -IP: IP from same subnet of running system. IP will be used while installing jails to download necessary content."
    echo " -INTERFACE: The name of the physical interface that will be used while connecting internet."

}

install_router() {
    # install bash, portaudit, , RTADVD, perl modules and softflowd
    # net-snmp with IPV6 and default SMUX support
    jexec $JAILID csh -c "pkg_add -r bash"
    jexec $JAILID csh -c "mkdir -p /usr/local/kovan/etc"

    jexec $JAILID csh -c "cd /usr/ports/ports-mgmt/portaudit && make install clean"
    jexec $JAILID csh -c "/usr/local/sbin/portaudit -Fda"

    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/net-snmp ; make -DBATCH -DWITH_IPV6 install clean"
    jexec $JAILID csh -c "cd /usr/ports/net/quagga  && make  -DBATCH -DWITH_RTADV -DWITH_SNMP install clean"

    jexec $JAILID csh -c "cd /usr/ports/textproc/p5-YAML-Tiny/;  make -DBATCH    -DFORCE_PKG_REGISTER install clean"
    jexec $JAILID csh -c "cd /usr/ports/devel/p5-Getopt-Long;  make -DBATCH  -DFORCE_PKG_REGISTER install clean"
    jexec $JAILID csh -c "cd  /usr/ports/net-mgmt/softflowd ;  make -DBATCH install clean"
}

install_node() {
    # install bash, portaudit and perl modules.
    jexec $JAILID csh -c "pkg_add -r bash"
    jexec $JAILID csh -c "mkdir -p /usr/local/kovan/etc"
    jexec $JAILID csh -c "cd /usr/ports/ports-mgmt/portaudit && make install clean"
    jexec $JAILID csh -c "/usr/local/sbin/portaudit -Fda"
    jexec $JAILID csh -c "cd /usr/ports/textproc/p5-YAML-Tiny/;  make -DBATCH    -DFORCE_PKG_REGISTER install clean"
    jexec $JAILID csh -c "cd /usr/ports/devel/p5-Getopt-Long;  make -DBATCH  -DFORCE_PKG_REGISTER install clean"
}

install_blackhole() {
    # install bash, portaudit, net-snmp, softflowd and perl modules.
    jexec $JAILID csh -c "pkg_add -r bash"
    jexec $JAILID csh -c "mkdir -p /usr/local/kovan/etc"
    jexec $JAILID csh -c "cd /usr/ports/ports-mgmt/portaudit && make install clean"
    jexec $JAILID csh -c "/usr/local/sbin/portaudit -Fda"
    jexec $JAILID csh -c "cd /usr/ports/textproc/p5-YAML-Tiny/;  make -DBATCH    -DFORCE_PKG_REGISTER install clean"
    jexec $JAILID csh -c "cd /usr/ports/devel/p5-Getopt-Long;  make -DBATCH  -DFORCE_PKG_REGISTER install clean"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/net-snmp  && make -DBATCH -DWITH_IPV6 install clean"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/softflowd ;  make -DBATCH install clean"
}

install_monitor() {
    jexec $JAILID csh -c "cd /usr/ports/ports-mgmt/portaudit && make install clean"
    jexec $JAILID csh -c "/usr/local/sbin/portaudit -Fda"

    jexec $JAILID csh -c "pkg_add -r bash"
    jexec $JAILID csh -c "mkdir -p /usr/local/kovan/etc"

    jexec $JAILID csh -c "cd /usr/ports/www/apache22 && make  -DBATCH install clean"
    jexec $JAILID csh -c "cd /usr/ports/lang/php5 && make -DBATCH -DWITH_APACHE -DWITH_IPV6"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/net-snmp  && make -DBATCH -DWITH_IPV6 install"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/nfsen &&  make  -DBATCH install clean"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/mrtg/ && make -DBATCH  -DWITH_IPV6 -DWITH_SNMP install clean"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/nagios-plugins  && make -DBATCH  -DWITH_JAIL -DWITH_IPV6 -DWITH_NETSNMP -DWITH_FPING  install clean"

    jexec $JAILID csh -c "/usr/sbin/pw groupadd nagios"
    jexec $JAILID csh -c "/usr/sbin/pw adduser nagios nagios"
    jexec $JAILID csh -c "cd /usr/ports/net-mgmt/nagios  && make -DBATCH  install clean"
    jexec $JAILID csh -c "cd /usr/ports/textproc/p5-YAML-Tiny/;  make -DBATCH install clean"
    jexec $JAILID csh -c "cd /usr/ports/devel/p5-Getopt-Long;  make -DBATCH  install clean"
    jexec $JAILID csh -c "cd /usr/ports/databases/mysql51-server ;  make -DBATCH  install clean"
    jexec $JAILID csh -c "cd /usr/ports/databases/libdbi-drivers ;  make -DBATCH  -DWITHOUT_PGSQL -DWITHOUT_SQLITE3 -DWITH_MYSQL  install clean"
    jexec $JAILID csh -c "cd /usr/ports/sysutils/syslog-ng3  ;  make -DBATCH  -DWITH_SQL -DWITH_IPV6 -DWITH_PCRE -DWITHOUT_SPOOF  install clean"

####For Syslog-ng 
    jexec $JAILID csh -c "/usr/local/etc/rc.d/mysql-server onestart"
    jexec $JAILID csh -c "echo \"create database kovan\" | mysql -uroot"
    jexec $JAILID csh -c "echo \"CREATE USER 'logger'@'localhost' IDENTIFIED BY '060112'\" | mysql -uroot "
    jexec $JAILID csh -c "echo \"GRANT ALL ON kovan.* TO 'logger'@'localhost'\" | mysql -uroot"
    jexec $JAILID csh -c "mysqladmin -u root password 060112"
    jexec $JAILID csh -c "/usr/local/etc/rc.d/mysql-server onestop"

### For graphs
    jexec $JAILID csh -c "cd /usr/ports/databases/p5-DBI ;  make -DBATCH  install clean"
    jexec $JAILID csh -c "cd /usr/ports/databases/p5-DBD-mysql ;  make -DBATCH  install clean"
    jexec $JAILID csh -c "cd /usr/ports/graphics/p5-GD-Graph ;  make -DBATCH  install clean"
}

install_jails() {

    echo "Jail IP adresi $IP"
    echo "Jail Ag Arabirimi $INT"
    ifconfig $INT $IP alias

    for i in "${JAILS[@]}"
        do
            echo "Generating Jail $i" 
            ezjail-admin  create -r $KVN_PREFIX/$i  $i $IP

            rm $KVN_PREFIX/$i/usr/ports
            mkdir $KVN_PREFIX/$i/usr/ports
            mount_nullfs -o ro /usr/ports $KVN_PREFIX/$i/usr/ports
            cp /etc/resolv.conf $KVN_PREFIX/$i/etc
            echo "WITHOUT_X11=YES" >>  $KVN_PREFIX/$i/etc/make.conf
            /usr/local/etc/rc.d/ezjail.sh onestart $i

            JAILID=`jls | grep $i | awk '{print $1}'`

            if [ $i == "router" ]
                then
                    install_router
            fi

            if [ $i == "node" ]
                then
                    install_node
            fi
            
            if [ $i == "monitor" ]
                then
                    install_monitor
            fi
            
            if [ $i == "blackhole" ]
                then
                    install_blackhole
            fi
            /usr/local/etc/rc.d/ezjail.sh onestop $i
            ezjail-admin delete $i
            umount $KVN_PREFIX/$i/usr/ports
            
        done
    ifconfig $INT $IP -alias

}


if [ $# -ne 3 ]; then
    install_usage
    exit 1
fi

KVN_PREFIX=$1
IP=$2
INT=$3

install_main

reboot
