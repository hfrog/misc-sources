class bashrc::and {
	file { 'and_.bashrc':
		path => '/home/and/.bashrc',
		ensure => file,
		owner => 'and',
		group => 'and',
		mode => '0644',
		source => "puppet:///modules/bashrc/.bashrc",
	}

	file { 'and_.bashrc_local':
		path => '/home/and/.bashrc_local',
		ensure => file,
		owner => 'and',
		group => 'and',
		mode => '0644',
		source => "puppet:///modules/bashrc/.bashrc_local",
	}

	file { 'and_.profile':
		path => '/home/and/.profile',
		ensure => file,
		owner => 'and',
		group => 'and',
		mode => '0644',
		source => "puppet:///modules/bashrc/.profile",
	}
}
