QOF - Query Object Framework
============================

Please see the NEWS file and the ChangeLog for detailed
information on changes since 0.6.0.

Building
--------
See the [README.vcs](README.vcs) for instructions on how to build this.

pkg-config support prior to libqof2
-----------------------------------

v0.7.4 includes 'qof.pc' in advance of libqof2 to ease the imminent
transition - packages can now check for 'qof' via pkg-config (instead
of 'qof-1') and still build against libqof2 when qof-1.pc is removed.

Documentation:
--------------
The QOF documentation is entirely embedded in the source code. You
can build an html version of the documentation by installing the
'doxygen' package, and then 'cd doc; make doc'.  Skim the doc/README
file for more info.
