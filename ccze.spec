%define version 0.2.1
%define dist stable
%define release 1

Summary: A robust log colorizer
Name: ccze
Version: %{version}
Release: %{release}
Vendor: Generic
Packager: Gergely Nagy <algernon@bonehunter.rulez.org>
URL: http://bonehunter.rulez.org/CCZE.html
Copyright: GPL
Group: Applications/Text
Source: ftp://bonehunter.rulez.org/pub/ccze/%{dist}/ccze-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
BuildPrereq: ncurses-devel >= 5.0, pcre-devel >= 3.1

%description
CCZE is a roboust and modular log colorizer, with plugins for apm,
exim, fetchmail, httpd, postfix, procmail, squid, syslog, ulogd,
vsftpd, xferlog and more.

%prep
%setup -q

%build
%configure --with-builtins=all
make

%install
%makeinstall
install -d %{buildroot}/%{_sysconfdir}
src/ccze-dump >%{buildroot}/%{_sysconfdir}/cczerc

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog ChangeLog-0.1 NEWS README THANKS FAQ
%config %{_sysconfdir}/cczerc
%{_bindir}/ccze
%{_bindir}/ccze-cssdump
%{_includedir}/ccze.h
%{_mandir}/man1/ccze.1*
%{_mandir}/man1/ccze-cssdump.1*
%{_mandir}/man7/ccze-plugin.7*

%changelog
* Sat Mar 29 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
- Support different distributions than CCZE stable.
* Tue Feb 25 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
- Include FAQ.
* Thu Feb 20 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
- Use %{buildroot}, %configure and %makeinstall.
- Improved description.
* Sat Jan 25 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
- Include %{_includedir}/ccze.h
* Mon Jan 13 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
- Include ccze-cssdump and its manual page.
- Packager changed to myself.
* Wed Nov 20 2002 Gergely Nagy <algernon@bonehunter.rulez.org>
- Build all modules statically.
* Tue Nov 19 2002 Gergely Nagy <algernon@bonehunter.rulez.org>
- Use src/ccze-dump to generate the default configuration file.
* Tue Nov 19 2002 Gergely Nagy <algernon@bonehunter.rulez.org>
- Simplified and made it version-neultral.
* Sun Nov 17 2002 Andreas Brenk <ab@aegisnet.biz>
- Initial creation
