Name:       huion-probe
Version:    2
Release:    1%{?dist}
Summary:    Huion graphics tablet probing utility

License:    GPLv2+
Source:     %{name}-%{version}.tar.gz

Requires:   libusbx
BuildRequires:	libusbx-devel

%description

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
%{_bindir}/%{name}

%changelog
