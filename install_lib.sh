#!bin/sh

SOURCE=$(dirname "${BASH_SOURCE[0]}")
instpath=$1

if [ "$#" -ne 1 ]; then
	instpath=$SOURCE'/../../build/usr/'
	install -d $instpath
    echo "Use default dir" $instpath
fi

incpath=$instpath"/include/"
libpath=$instpath"/lib/"
install -d $incpath
install -d $libpath

rsync -auzv --delete-before --include '*.h' --exclude '*.cxx' --exclude '*.txt' --exclude '.*' $SOURCE $incpath

SO_SOURCE=$SOURCE"/../../build"

cp -aRd $SO_SOURCE'/libcnoda.so' $SO_SOURCE'/libcnoda.so.1' $libpath
#rsync -auzv --delete-before --include '*.so' --exclude '*' $SO_SOURCE $libpath