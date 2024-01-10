%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    gcom modbus interface
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, cjson, libmodbus

%description
GCOM Modbus Interface to FIMS

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 release/gcom_modbus_server %{buildroot}%{bin_dir}
install -m 0755 release/gcom_modbus_client %{buildroot}%{bin_dir}

install -m 0644 release/gcom_modbus_client@.service %{buildroot}%{systemd_dir}
install -m 0644 release/gcom_modbus_server@.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/gcom_modbus_server
%{bin_dir}/gcom_modbus_client
%{systemd_dir}/gcom_modbus_client@.service
%{systemd_dir}/gcom_modbus_server@.service

%changelog