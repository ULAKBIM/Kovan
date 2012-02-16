#!/usr/local/bin/bash -x 

set -e 
export PACKAGEROOT="ftp://ftp.ulak.net.tr"
pkg_add -r cvsup-without-gui

echo "*default host=tulumba.ulakbim.gov.tr" > /etc/stable-supfile
echo "*default base=/usr" >> /etc/stable-supfile
echo "*default prefix=/usr" >> /etc/stable-supfile 
echo "*default release=cvs tag=RELENG_8_1" >> /etc/stable-supfile
echo "*default delete use-rel-suffix " >> /etc/stable-supfile
echo "src-all" >> /etc/stable-supfile

cvsup /etc/stable-supfile 


portsnap -s portsnap2.freebsd.org fetch
portsnap extract
    
echo "Kernel compile"
echo "options VIMAGE" >> /usr/src/sys/amd64/conf/GENERIC
cd /usr/src/sys/amd64/conf
config GENERIC
cd ../compile/GENERIC/
make  -j4 cleandepend && make -j4  depend && make -j4  && make install


FILENAME='/etc/rc.conf'
if grep 'ipv6_enable="YES"' $FILENAME >/dev/null 2>&1
then
    echo 'ipv6_enable="YES" exists.'
else
    echo 'ipv6_enable="YES"' >> $FILENAME
    echo 'ipv6_enable statement written succesfully.'
fi

echo "net.inet.ip.forwarding=1" >> /etc/sysctl.conf
echo "net.inet6.ip6.forwarding=1" >> /etc/sysctl.conf
echo "security.jail.allow_raw_sockets=1" >> /etc/sysctl.conf


echo "IPC ayarlari" 


echo "kern.ipc.shmall=32768" >> /boot/loader.conf
echo "kern.ipc.shmmax=134217728" >> /boot/loader.conf
echo "kern.ipc.semmap=256" >> /boot/loader.conf
echo "kern.ipc.semmni=256" >> /boot/loader.conf
echo "kern.ipc.semmns=512" >> /boot/loader.conf
echo "kern.ipc.semmnu=256" >> /boot/loader.conf



cd /usr/ports/sysutils/ezjail
make -DBATCH install clean
ezjail-admin install
    
cd /usr/ports/lang/perl5.10 && make -DBATCH install clean
cd /usr/ports/textproc/p5-YAML-Tiny/ &&   make -DBATCH    install clean
cd /usr/ports/devel/p5-Getopt-Long &&   make -DBATCH  install clean
export CONFIGURE_ARGS="--enable-ipv6 --enable-gre --enable-targetbased --enable-decoder-preprocessor-rules --enable-zlib"
cd /usr/ports/devel/automake111 && make -DBATCH install clean
cd /usr/ports/security/snort && make -DBATCH -DWITH_SNORTSAM install clean

reboot
	




