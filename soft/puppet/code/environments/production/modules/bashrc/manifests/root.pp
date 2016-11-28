class bashrc::root {
	file { 'root_.bashrc':
		path => '/root/.bashrc',
		ensure => file,
		owner => 'root',
		group => 'root',
		mode => '0644',
		source => "puppet:///modules/bashrc/.bashrc",
	}

	file { 'root_.bashrc_local':
		path => '/root/.bashrc_local',
		ensure => file,
		owner => 'root',
		group => 'root',
		mode => '0644',
		source => "puppet:///modules/bashrc/.bashrc_local",
	}

	file { 'root_.profile':
		path => '/root/.profile',
		ensure => file,
		owner => 'root',
		group => 'root',
		mode => '0644',
		source => "puppet:///modules/bashrc/.profile",
	}
}
