### Create autotools files (developer only):
# create all the Makefile.am files necessary
autoscan
mv configure.scan configure.ac
# edit configure.ac
#   add AM_INIT_AUTOMAKE() after AC_INIT
#   add LT_INIT before AC_PROG_CC
#   comment AC_CONFIG_HEADERS([config.h])
#   comment AC_CHECK_LIB([dl], [main])
#   edit:
#     AC_INIT([memcheck], [1.0], [BUG-REPORT-ADDRESS])

### configure, compile and install:
libtoolize
aclocal
automake --add-missing --foreign
autoconf
./configure --prefix=<install_dir>
make
make install

### run:
<install_dir>/bin/memcheck [options]

### clean up:
make uninstall
make clean
rm -rf aclocal.m4 autom4te.cache compile config.guess config.log config.status config.sub configure depcomp install-sh libtool ltmain.sh Makefile Makefile.in missing */.deps */Makefile */Makefile.in

