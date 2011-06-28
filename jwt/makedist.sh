#!/bin/sh

version=$1
[ x$version = x ] && echo "Version missing!" && exit 1

path=$(pwd)
dest=/tmp/jwt-${version}

rm -rf $dest
mkdir $dest $dest/pali
cp pali/*.js $dest/pali
cp changelog.txt copying.txt $dest 
cp jwt.js dialog.js style.css $dest
cat readme.txt | sed "s/#VERSION#/${version}/g" > $dest/readme.txt
cat pali.htm | sed "s/#VERSION#/${version}/g" > $dest/pali.htm

rm -f $path/jwt-${version}.zip
cd /tmp
zip -r $path/jwt-${version}.zip jwt-${version}
rm -rf $dest
exit 0
