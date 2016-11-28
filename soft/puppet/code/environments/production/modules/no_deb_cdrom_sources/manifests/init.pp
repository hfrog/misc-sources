class no_deb_cdrom_sources {
	file_line { 'deb_cdrom':
		ensure => absent,
		path => '/etc/apt/sources.list',
		line => 'deb cdrom',
		match => '^\s*deb\s+cdrom.*',
		match_for_absence => true,
		replace => false,
		multiple => true,
	}
}
