node default {

# disable cdrom in /etc/apt/sources.list
	class { 'no_deb_cdrom_sources': }

# sudo
	class { 'sudo': }
	sudo::conf { 'sudo':
		content => "%sudo ALL=(ALL:ALL) NOPASSWD: ALL",
	}
	class { 'privileges': }
	sudo::conf { 'and':
		priority => 60,
		content => "and ALL=(ALL:ALL) NOPASSWD: ALL",
	}

# profile
	class { 'bashrc::and': }
	class { 'bashrc::root': }

# disable unused services
# XXX to be corrected
#	case $facts['os']['distro']['codename'] {
#		'jessie': {
#			class { 'stopped_exim4': }
#			class { 'stopped_rpcbind': }
#		}
#	}

# packages
	class { 'apt_get_update': }
	class { 'curl': }
	class { 'vmtools': }
	class { 'gpm': }
	class { 'ssh': }
	class { 'bridge_utils': }

# docker
	include 'docker'
	ssh_authorized_key { 'and@debian-conf':
		user => 'and',
		type => 'ssh-rsa',
		key => 'xxx key here',
	}
}
