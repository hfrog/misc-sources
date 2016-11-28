class ssh {
	package { "openssh-server":
		ensure => "latest",
	}

	package { "openssh-client":
		ensure => "latest",
	}
}
