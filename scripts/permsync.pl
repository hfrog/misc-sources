#!/usr/bin/perl -w
#
# $Id: permsync.pl,v 1.1 2006/07/05 12:02:49 root Exp $
#
# Sync perms/owner/groups of two dirs from one to other
#

use strict;
use Getopt::Std;
use Time::localtime;
use File::stat;

my %opts;
getopts('nv', \%opts);

my ($src, $dst) = (undef, undef);
if (defined($ARGV[0]) and defined($ARGV[1])) {
    $src = $ARGV[0];
    $dst = $ARGV[1];
    logger("sync $src -> $dst");
} else {
    print STDERR ("Usage: permsync.pl [-nv] src dst\n");
    exit(1);
}

sync_dir($src, $dst);

sub sync_item {
    my ($src_item, $dstdir) = @_;
    my ($st, $d_st);
    (my $dst_item = $src_item) =~ s/^$src/$dst/;

    if (-e $src_item) {
	if (-l $src_item) {
	    $st = lstat($src_item) or die "Error: can't stat $src_item: $!\n";
	} else {
	    $st = stat($src_item) or die "Error: can't stat $src_item: $!\n";
	}
#	if (!defined($st)) {
#	    warn "Warning: skipping $src_item, can't stat: $!\n";
#	    return;
#	}
    } else {
	warn "Warning: skipping $src_item: no such file or directory.\n";
	return;
    }

    if (-e $dst_item) {
	if (-l $dst_item) {
	    $d_st = lstat($dst_item) or die "Error: can't stat $dst_item: $!\n";
	} else {
	    $d_st = stat($dst_item) or die "Error: can't stat $dst_item: $!\n";
	}
	if (!defined($d_st)) {
	    warn "Warning: skipping $dst_item, can't stat: $!\n";
	    return;
	}
    } else {
	warn "Warning: skipping $dst_item: no such file or directory.\n";
	return;
    }

    if ($st->mode != $d_st->mode) {
	logger("mode  $dst_item " . $d_st->mode . " -> " . $st->mode);
	unless (exists($opts{'n'}) and defined($opts{'n'})) {
	    chmod($st->mode, $dst_item) or
			die "Error: can't chmod $dst_item: $!\n";
	}
    }
    if ($st->uid != $d_st->uid or $st->gid != $d_st->gid) {
	if ($st->uid != $d_st->uid) {
	    logger("user  $dst_item " . $d_st->uid . " -> " . $st->uid);
	}
	if ($st->gid != $d_st->gid) {
	    logger("group $dst_item " . $d_st->gid . " -> " . $st->gid);
	}
	unless (exists($opts{'n'}) and defined($opts{'n'})) {
	    chown($st->uid, $st->gid, $dst_item) or
			die "Error: can't chown $dst_item: $!\n";
	}
    }
}

sub sync_dir {
    my ($dir, $dstdir) = @_;
    opendir(D, $dir) or die "Error: can't open dir $dir: $!\n";
    my @dirs   = grep { !/^\.$/ and !/^\.\.$/ and -d "$dir/$_" } readdir(D);
    rewinddir(D) or die "Error: can't rewind dir $dir: $!\n";
    my @files  = grep { -f "$dir/$_" or -l "$dir/$_" } readdir(D);
    rewinddir(D) or die "Error: can't rewind dir $dir: $!\n";
    my @others = grep { ! -d "$dir/$_" and ! -f "$dir/$_" and ! -l "$dir/$_" }
		readdir(D);
    closedir(D) or warn "Warning: can't close dir $dir: $!\n";

    foreach (@dirs) {
	sync_dir("$dir/$_", $dstdir);
    }

    # handle files and the directory itself
    foreach ((@files, ".")) {
	sync_item("$dir/$_", $dstdir);
    }

    if (scalar(@others)) {
	print STDERR ("Warning: no-file(s) found: " .
		join(" ", map { "$dir/$_" } @others, "\n"));
    }
}

BEGIN {
    sub logger {
	return unless (exists($opts{'v'}) and defined($opts{'v'}));
	my $datestamp = sprintf("%02d:%02d:%02d", localtime->hour(),
		localtime->min(), localtime->sec());
	print STDERR ("[ $datestamp ", @_, " ]\n");
    }
}


__END__

ok 1. parse argv and get src and dst;
ok 2. get content of src, recursively digging in until no dir found inside;
ok 3. for each file:
ok     - look at the same dst file;
ok     - chmod;
ok     - chgrp;
ok     - chown;

ok correctly handle symlinks, but don't follow them

