### Create autotools files (developer only):
# create all the Makefile.am files necessary
autoscan
mv configure.scan configure.ac
# edit configure.ac
#   add AM_INIT_AUTOMAKE() after AC_INIT
#   comment AC_CONFIG_HEADERS([config.h])
#   edit:
#     AC_INIT([dbugger], [1.0], [BUG-REPORT-ADDRESS])

### configure, compile and install:
aclocal
automake --add-missing --foreign
autoconf
./configure --prefix=<install_dir>
make
make install

### run:
<install_dir>/bin/dbugger [options]

### clean up:
make uninstall
make clean
rm -rf aclocal.m4 autom4te.cache compile config.guess config.log config.status config.sub configure depcomp install-sh libtool ltmain.sh Makefile Makefile.in missing */.deps */Makefile */Makefile.in autom4te.cache autoscan.log configure.scan
