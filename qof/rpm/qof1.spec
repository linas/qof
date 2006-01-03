%define name qof
%define version 0.6.1
%define release 1

Summary: Query Object Framework
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Libraries
Source: http://prdownloads.sourceforge.net/qof/qof-0.6.1.tar.gz
URL: http://qof.sourceforge.net/
# Remember to change the packager string to your details
Packager: Neil Williams
Prefix: %{_prefix} 
Requires: libxml2 >= 2-2.5.10, glib2
%description 
A framework to allow the execution of SQL 
queries using collections of in-memory objects as 'tables'.

This package provides the shared libraries needed to run programs
developed using QOF, including the QSF backend module.

%package devel
Summary: The development headers for QOF
Group: Development/Libraries
Requires: %{name} = %{version}
%description devel
A framework to allow the execution of SQL queries using collections 
of in-memory objects as 'tables'.

This package provides the header files needed to develop 
applications using QOF.

%prep
%setup -q
%build 
%configure
make

%install
%makeinstall

%clean
make uninstall

# Keep the .la in the main package to help
# portability of the QSF backend.
# libqofsql is only needed if libgda is not available.

# If you re-package on a system with libgda, (FC4)
# add a libgda (>= 1.2.0) dependency.
%files
%doc AUTHORS README NEWS ChangeLog
%{_libdir}/libqof.la
%{_libdir}/libqof.so
%{_libdir}/libqof.so.1.0.2
%{_libdir}/libqof.so.1
%{_libdir}/libqof-backend-qsf.la
%{_libdir}/libqof-backend-qsf.so
%{_libdir}/libqof-backend-qsf.so.0.0.1
%{_libdir}/libqof-backend-qsf.so.0
%{_libdir}/libqofsql.la
%{_libdir}/libqofsql.so
%{_libdir}/libqofsql.so.1.0.1
%{_libdir}/libqofsql.so.1
%{_datadir}/locale/en_GB/LC_MESSAGES/qof.mo
%{_datadir}/xml/qof/qsf/pilot-qsf-GnuCashInvoice.xml
%{_datadir}/xml/qof/qsf/qsf-map.xsd.xml
%{_datadir}/xml/qof/qsf/qsf-object.xsd.xml

%files devel
%defattr(-,root,root)
%doc AUTHORS INSTALL README NEWS ChangeLog TODO
%{_libdir}/libqof.a
%{_libdir}/libqofsql.a
%{_libdir}/libqof-backend-qsf.a
%{_libdir}/pkgconfig/qof-1.pc
%{_includedir}/qof/deprecated.h
%{_includedir}/qof/gnc-date.h
%{_includedir}/qof/gnc-engine-util.h
%{_includedir}/qof/gnc-event.h
%{_includedir}/qof/gnc-numeric.h
%{_includedir}/qof/guid.h
%{_includedir}/qof/kvp_frame.h
%{_includedir}/qof/kvp-util.h
%{_includedir}/qof/kvp-util-p.h
%{_includedir}/qof/qofbackend.h
%{_includedir}/qof/qofbackend-p.h
%{_includedir}/qof/qof-backend-qsf.h
%{_includedir}/qof/qof-be-utils.h
%{_includedir}/qof/qofla-dir.h
%{_includedir}/qof/qofbook.h
%{_includedir}/qof/qof_book_merge.h
%{_includedir}/qof/qofclass.h
%{_includedir}/qof/qofchoice.h
%{_includedir}/qof/qofgobj.h
%{_includedir}/qof/qof.h
%{_includedir}/qof/qofid.h
%{_includedir}/qof/qofid-p.h
%{_includedir}/qof/qofinstance.h
%{_includedir}/qof/qofinstance-p.h
%{_includedir}/qof/qoflog.h
%{_includedir}/qof/qofobject.h
%{_includedir}/qof/qofquerycore.h
%{_includedir}/qof/qofquery.h
%{_includedir}/qof/qofsession.h
%{_includedir}/qof/qofsql.h
%{_includedir}/qof/sql_parser.h

# If libgda is used, qof/sql_parser is not
# installable.
