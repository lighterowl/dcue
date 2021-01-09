#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
  echo >&2 "Usage : $0 test_binary data_dir"
  exit 1
fi

test_bin=$(readlink -f "$1")
data_dir=$(readlink -f "$2")

wd=$(mktemp -d)
cd "$wd"
prevwd=$OLDPWD

num_fails=0
for i in "$data_dir"/*.json; do
  fname=${i##*/}
  fname_noext=${fname%.*}

  mkdir "$fname_noext"
  pushd "$fname_noext" >/dev/null 2>&1

  echo >&2 "Testing $fname_noext..."
  "$test_bin" "$i" "${fname_noext}.wav"
  diff -r "${data_dir}/${fname_noext}" -I '^REM COMMENT' .
  [[ $? -ne 0 ]] && num_fails=$((num_fails + 1))

  popd >/dev/null 2>&1
done

cd "$prevwd"
rm -fR "$wd"
exit $num_fails
