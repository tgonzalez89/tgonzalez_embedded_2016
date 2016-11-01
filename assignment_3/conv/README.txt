### Create autotools files (developer only):
# create all the Makefile.am files necessary
autoscan
mv configure.scan configure.ac
# edit configure.ac
#   add AM_INIT_AUTOMAKE() after AC_INIT
#   comment AC_CONFIG_HEADERS([config.h])
#   edit:
#     AC_INIT([conv], [1.0], [BUG-REPORT-ADDRESS])
#   add before AC_PROG_CC:
#     AC_CHECK_PROGS([DOXYGEN], [doxygen])
#     if test -z "$DOXYGEN";
#        then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
#     fi
#
#     AM_CONDITIONAL([HAVE_DOXYGEN],
#     [test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])


### configure, compile and install:
aclocal
automake --add-missing --foreign
autoconf
./configure --prefix=<install_dir>
# CFLAGS: -O3 to make it run faster (don't use with -pg)
#         -pg to get timing info with gprof
#         -fopenmp -DUSEOPENMP to run on multiple cores
make CFLAGS=""
make install

### documentation on ./doc/html

### run:
<install_dir>/bin/conv [options]

### clean up:
make uninstall
make clean
rm -rf aclocal.m4 autom4te.cache compile config.guess config.log config.status config.sub configure depcomp install-sh libtool ltmain.sh Makefile Makefile.in missing */.deps */Makefile */Makefile.in autom4te.cache autoscan.log configure.scan */Doxyfile
