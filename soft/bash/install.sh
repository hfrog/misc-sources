#!/bin/bash
# vim: set sw=2 :

if [[ -z ${1+x} ]]; then
  echo "Usage: $0 <dst_dir>"
  exit 1
fi
dst_dir=$1

for f in dot*; do
  dst=$dst_dir/${f#dot}
  rm -f $dst
  cp -v $f $dst
done
