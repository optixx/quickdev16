Summary:   Library to program and control the FTDI USB controller
Name:      libftdi
Version:   0.7
Release:   1
Copyright: LGPL
Group:     System Environment/Libraries
Vendor:    Intra2net AG
Source:    %{name}-%{version}.tar.gz
Buildroot: /tmp/%{name}-%{version}-root
Requires:  libusb
BuildRequires: libusb, libusb-devel, pkgconfig
Prefix:    /usr

%package   devel
Summary:   Header files and static libraries for libftdi
Group:     Development/Libraries
Requires:  libftdi = %{version}, libusb-devel

%description 
Library to program and control the FTDI USB controller

%description devel
Header files and static libraries for libftdi

%prep
%setup -q

%build
./configure --prefix=%{prefix}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -fr $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING.LIB
%{prefix}/lib/libftdi.so*

%files devel
%defattr(-,root,root)
%{prefix}/bin/libftdi-config
%{prefix}/lib/libftdi.*a
%{prefix}/include/*.h
%{prefix}/lib/pkgconfig/*.pc
