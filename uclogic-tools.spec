Name:       uclogic-tools
Version:    3
Release:    1%{?dist}
Summary:    UC-Logic graphics tablet diagnostic tools

License:    GPLv2+
Source:     %{name}-%{version}.tar.gz

Requires:   libusbx
BuildRequires:	libusbx-devel

%description
 Uclogic-tools is a collection of programs for collecting and analyzing
 diagnostic information from UC-Logic (and rebranded) graphics tablets.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%files
%doc
%{_defaultdocdir}/%{name}
%{_bindir}/uclogic-*

%changelog
