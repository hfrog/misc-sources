class stopped_exim4 {
	service { 'exim4':
		ensure => 'stopped',
	}
}
