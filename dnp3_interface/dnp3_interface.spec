%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system
%define debug_package %{nil}

Summary:    dnp3 interface
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Provides:   libdmap.so()(64bit) 
Requires:   fims opendnp3

%description
DNP3 Interface that wraps opendnp3

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}
install --directory %{buildroot}%{lib_dir}

install -m 0755 dnp3_client %{buildroot}%{bin_dir}
install -m 0755 dnp3_server %{buildroot}%{bin_dir}

install -m 0755 libdmap.so %{buildroot}%{lib_dir}

install -m 0644 dnp3_client@.service %{buildroot}%{systemd_dir}
install -m 0644 dnp3_server@.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{lib_dir}/libdmap.so
%{bin_dir}/dnp3_client
%{bin_dir}/dnp3_server
%{systemd_dir}/dnp3_client@.service
%{systemd_dir}/dnp3_server@.service

%changelog
