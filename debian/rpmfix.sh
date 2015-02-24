#
#rpmfix.sh 2014-09-25
#
#The problem:
#After 'alien --to-rpm pkgfile.deb' RPM wcm package claims
#ownership to several system folders: / /usr /usr/bin et.al.
#This breaks installation on Fedora, by raising
#file conflict with 'filesystem' package, which owns
#these files.
#
#The script extracts data from RRM, and repackages 
#it in RPM-native way, i.e. with 'rpmbuild' tool using 
#custom spec file. The spec file should have file list without 
#'/', and other common folders.
#
#

if [ -z $2 ]
then
echo Usage $0 packagefile.rpm specfile.spec
exit 1
fi
 
PKG=$1
PKG=`echo $PKG | sed 's/\.rpm$//'`
if [ ! -f ${PKG}.rpm ]
then
echo missing  ${PKG}.rpm
exit 2
fi


#SPECFILE=`echo ${PKG}| sed s/\.[^\.]*$//`.spec
SPECFILE=$2

if [ ! -f ${SPECFILE} ]
then
echo "missing spec file: " $SPECFILE
exit 3
fi

#PKGDIR=~/rpmbuild/BUILDROOT/${PKG}
PKGDIR=`pwd`/PKG
ARCH=`echo ${PKG} | sed 's/^.*\.//g'`

echo rebuilding $PKG on arch $ARCH
rpm2cpio ${PKG}.rpm | \
( \
rm -rf ${PKGDIR} && \
mkdir -p ${PKGDIR} && \
cd ${PKGDIR} && \
cpio -idmv \
) && \
rpmbuild -bb  ${SPECFILE} --buildroot ${PKGDIR} --target $ARCH && \
echo Done. Fixed RPM is: ${HOME}/rpmbuild/RPMS/$ARCH/${PKG}.rpm
