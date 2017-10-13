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

if [[ -z ${1+x} ]]; then
  echo "Usage: $0 <dst_dir>"
  exit 1
fi
dst_dir=$1

for f in $(dirname $BASH_SOURCE)/dot*; do
  [ -f $f ]
  d=$(basename $f)
  dst=$dst_dir/${d#dot}
  rm -f $dst
  cp -v $f $dst
done
