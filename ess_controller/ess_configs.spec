%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define cfg_dir /usr/local/etc/config
%define local_cfg_dir configs/ess_controller
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    ess configs
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, cjson

%description
Ess Controller Configs

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{cfg_dir}/ess_controller

install -m 0755 ess_controller %{buildroot}%{cfg_dir}/ess_controller


%clean
rm -rf %{buildroot}

%files
%{local_cfg_dir}/ess_controller

%changelog