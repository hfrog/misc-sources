class privileges {
	sudo::conf { 'admins':
		ensure => present,
		content => '%admin ALL=(ALL) ALL',
	}
}
