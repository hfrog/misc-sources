class apt_get_update {
        exec { 'apt_get_update':
                command => 'apt-get update',
                path => '/usr/local/bin:/usr/bin:/bin',
        }
}
