#!/bin/bash
# vim: set sw=2 :

set -o errexit
set -o errtrace
set -o nounset
set -o pipefail

error_report() {
  echo "errexit on line $(caller)" >&2
}

trap error_report ERR

DEBUG=${DEBUG:-false}
if [[ $DEBUG == true ]]; then
  set -x
fi

TMP_CONF='/tmp/cfg.tmp'
echo "$DEX_CONFIG" >$TMP_CONF

# add '\' before '&' to avoid sed backref
for v in DEX__LDAP_BIND_PW DEX__STATIC_KUBERNETES_SECRET; do
  declare -n xv=$v
  if echo "$xv" | grep -q '&'; then
    xv=$(echo $xv | sed -e 's & \\& g')
  fi
done

sed -i s/__DEX__LDAP_HOST__/${DEX__LDAP_HOST:-}/g $TMP_CONF
sed -i s/__DEX__LDAP_BIND_DN__/${DEX__LDAP_BIND_DN:-}/g $TMP_CONF
sed -i "s __DEX__LDAP_BIND_PW__ ${DEX__LDAP_BIND_PW:-} g" $TMP_CONF
sed -i s/__DEX__LDAP_USERSEARCH_BASE_DN__/${DEX__LDAP_USERSEARCH_BASE_DN:-}/g $TMP_CONF
sed -i s/__DEX__LDAP_GROUPSERSEARCH_BASE_DN__/${DEX__LDAP_GROUPSERSEARCH_BASE_DN}/g $TMP_CONF

sed -i s/__DEX__STATIC_KUBERNETES_SECRET__/${DEX__STATIC_KUBERNETES_SECRET}/g $TMP_CONF

CONF=${DEX_CONFIG_FILE:-/etc/dex/cfg/config.yaml}
mkdir -p $(dirname $CONF)
mv -f $TMP_CONF $CONF
echo "Wrote DEX config $CONF"
if [[ $DEBUG == true ]]; then
  cat $CONF
fi
