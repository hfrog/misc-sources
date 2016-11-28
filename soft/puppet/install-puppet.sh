#!/bin/sh

os_release_file=/etc/os-release

puppetlabs_release=puppetlabs-release-pc1
puppet_server=debian-conf.localdomain

[ -f $os_release_file ] \
        && . /etc/os-release \
        || ( echo "Error loading $os_release_file" && exit 1)

#echo $ID
#echo $VERSION_ID

case $ID in
    debian)
        case $VERSION_ID in
            8)
                release=jessie
		puppet_conf=/etc/puppetlabs/puppet/puppet.conf
                ;;

            *)
                echo "Error: unrecognized $ID version"
                exit 1
                ;;
        esac
        ;;

    ubuntu)
        case $VERSION_ID in
            14.04)
                release=trusty
		puppet_conf=/etc/puppet/puppet.conf
                ;;

            *)
                echo "Error: unrecognized $ID version"
                exit 1
                ;;
        esac
        ;;

    *)
        echo 'Error: unrecognized release'
        exit 1
        ;;
esac

echo release=$release
echo puppet.conf=$puppet_conf

debfile=$puppetlabs_release-$release.deb

# install puppet package
if dpkg -l | grep -qw puppet; then
    : # puppet-agent is already installed
else
    echo install $debfile
    if dpkg -l | grep -q $puppetlabs_release; then
	: # $puppetlabs_release is already installed
    else
	[ -f $debfile ] || wget https://apt.puppetlabs.com/$debfile
	[ -f $debfile ] && dpkg -i $debfile || exit 1
    fi
    apt-get update \
	&& apt-get -y install puppet
fi

# update /etc/hosts
if grep -q debian-conf /etc/hosts; then
    : # master is already written in /etc/hosts
else
    echo adding $puppet_server to /etc/hosts
    echo '192.168.209.129 debian-conf debian-conf.localdomain' >> /etc/hosts
fi

# update puppet.conf
if grep -q $puppet_server $puppet_conf; then
    : # master is already configured in $puppet_conf
else
    echo adding $puppet_server to $puppet_conf
    echo >> $puppet_conf
    echo '[agent]' >> $puppet_conf
    echo "server=$puppet_server" >> $puppet_conf
fi

