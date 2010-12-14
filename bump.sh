#!/bin/sh

sed -r -i 's/AC_INIT\(\[kbdd\],\ \[([0-9]|\.)+], \[qnikst@gentoo.ru\]\)/AC_INIT\(\[kbdd\],\ \['$1'],\ \[qnikst\@gentoo.ru\]\)/g' configure.ac
echo -e "$(date +%F)\tv$1 vesion bump\n$(cat ChangeLog)" > ChangeLog
git tag -s "v$1" -m "$1 version bump"
