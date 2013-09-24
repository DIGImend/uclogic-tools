Name:       huion-tools
Version:    2
Release:    1%{?dist}
Summary:    Huion graphics tablet diagnostic tools

License:    GPLv2+
Source:     %{name}-%{version}.tar.gz

Requires:   libusbx
BuildRequires:	libusbx-devel

%description
 Huion-tools is a collection of programs for collecting and analyzing
 diagnostic information from Huion graphics tablets.

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
%{_bindir}/huion-*

%changelog
