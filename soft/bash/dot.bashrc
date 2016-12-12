#
# $Id: .bashrc,v 1.3 2016/03/23 15:34:47 root Exp $
# $Template: 1.19 $
#
os ()
{
    uname | tr '[:upper:]' '[:lower:]'
}
if [ -z "$HOST" ]; then
    export HOST=$(hostname)
fi
export PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/X11R6/bin
case `os` in
    sunos)
	PATH=$(getconf PATH):$PATH
    ;;
esac
if [ `id -u` = "0" ]; then
    export HOME=/root
fi
export TERM=${TERM:-cons25}
export BLOCKSIZE=K
export EDITOR=vi
export PAGER=less
export LC_ALL=C
export LANG=C
umask 022
alias h='history 25'
alias j='jobs -l'
alias ss='sudo bash'
alias dt='dmesg | tail -20'
export LESS='-inRS'
export LESSCHARSET=koi8-r
#export LESSOPEN="|lesspipe.sh %s"
export EXINIT='set ai nows sw=4 et'
if [ -t 0 ] && [[ $- == *i* ]]; then
    stty erase 
    bind C-e:end-of-line
    case `os` in
	freebsd|openbsd|dragonfly)
	    stty discard undef
	    alias psap='ps -axww | grep http | grep s\ '
	    alias ll='ls -lFo'
	    EXINIT=${EXINIT:+${EXINIT}'|set cedit= noic iclower'}
	    ;;
	linux)
	    alias ll='ls -lF'
	    alias psap='ps axww | grep http | grep s\ '
	    alias hd='hexdump -C'
	    EXINIT=${EXINIT:+${EXINIT}'|set cedit= ic'}
	    ;;
	sunos)
	    # stty -icanon is buggy, set it by hands when you want
	    stty flush undef
	    alias ll='ls -lF'
	    EXINIT=${EXINIT:+${EXINIT}'|set ic'}
    esac
fi
if [ -n "$PS1" ]; then
    set +o noclobber
    set -o ignoreeof
    if [ `id -u` = "0" ]; then
	export PS1="\h# "
    else
	export PS1="\h: {\!} "
    fi
fi
export IGNOREEOF=0
export HISTCONTROL=ignoreboth
#export HISTTIMEFORMAT=1
choose_logfile ()
{
    logdirs='/var/log /var/adm /var/cron'
    logsufs='.log log'
    archsufs='.gz .bz2'

    name=$(echo $1 | sed -e 's/[0-9][0-9]*$//')
    suf=${1##$name}
    file=

    for s in "" $suf; do
	for ls in "" $logsufs; do
	    for as in "" $archsufs; do
		#     empty s,     empty suf: $name$ls$as == $name$suf$ls$as
		#     empty s, non-empty suf: $name$suf$ls$as
		# non-empty s,     empty suf: should never happens
		# non-empty s, non-empty suf: $name$ls.$suf$as
		if [ -z "$s" ]; then
		    f="$name$suf$ls$as"
		else
		    f="$name$ls.$suf$as"
		fi
		for ld in $logdirs; do
		    if [ -f "$ld/$f" ]; then
			file="$ld/$f"
			break
		    fi
		done
	    done
	done
    done

    if [ -z "$file" ]; then
	echo "logfiles like ${1} in $logdirs were not found." >&2
    fi
    echo "$file"
}
llog ()
{
    logfile=`choose_logfile "$1"`
    [ "$logfile" != "" ] && $PAGER +G "$logfile"
}
tlog ()
{
    logfile=`choose_logfile "$1"`
    [ "$logfile" != "" ] && tail "$logfile"
}
x509 ()
{
    openssl x509 -text -noout -in "$1"
}
csr ()
{
    openssl req -text -noout -in "$1"
}
stelnet ()
{
    openssl s_client -connect "$1":"$2"
}
fr ()
{
    find . -type f | xargs grep -li "$1"
}
lw ()
{
    ll $(which "$1")
}
if [ -n "$USER" ]; then
    h=`eval echo "~""$USER"`
    [ -f $h/.bashrc_local ] && . $h/.bashrc_local
fi
