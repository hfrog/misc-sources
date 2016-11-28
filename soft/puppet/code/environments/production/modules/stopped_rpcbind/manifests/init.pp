class stopped_rpcbind {
	service { 'rpcbind':
		ensure => 'stopped',
	}
}
