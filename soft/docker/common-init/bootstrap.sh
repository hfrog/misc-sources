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

echo "$script" > script
chmod +x script
exec ./script
