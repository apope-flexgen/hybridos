%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    gcom dnp3 interface
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, cjson

%description
Gcom DNP3 Interface to FIMS

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 gcom_dnp3_server %{buildroot}%{bin_dir}

install -m 0644 gcom_dnp3_server@.service %{buildroot}%{systemd_dir}

install -m 0755 gcom_dnp3_client %{buildroot}%{bin_dir}

install -m 0644 gcom_dnp3_client@.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/gcom_dnp3_server
%{systemd_dir}/gcom_dnp3_server@.service
%{bin_dir}/gcom_dnp3_client
%{systemd_dir}/gcom_dnp3_client@.service

%changelog