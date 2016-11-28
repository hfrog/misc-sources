divert(-1)
# $Id: stripped.mc,v 1.5 2004/08/11 15:45:35 rojer Exp $
# add small comment into the .cf file
# NOTE: after making a .cf file, you need to comment out the 'CE root' string
divert(0)dnl

##
## Look in the /etc/mail/ for the .mc file
##

VERSIONID(`@(#)stripped.mc 8.12 (domain.ru) 2006/02/03')dnl
OSTYPE(`bsd4.4')dnl
FEATURE(`nouucp', `reject')dnl
FEATURE(`nocanonify')dnl
FEATURE(`masquerade_envelope')dnl
MASQUERADE_AS(`domain.ru')dnl
#rcpt to: <@relay.local:and@hfrog.ru>
define(`MAIL_HUB', `relay.local')dnl
define(`SMART_HOST', `relay.local')dnl
define(`confBIND_OPTS', `-DNSRCH -DEFNAMES')dnl
define(`LOCAL_MAILER_MAX', `11000000')dnl
define(`confMAX_MESSAGE_SIZE', `11000000')dnl
define(`confQUEUE_LA', `40')dnl
define(`confTO_IDENT', `0')dnl
MAILER(smtp)dnl
