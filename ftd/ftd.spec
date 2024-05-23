%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    ftd
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
Archives FIMS messages to disk

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 ftd %{buildroot}%{bin_dir}
install -m 0755 fims_format_scan %{buildroot}%{bin_dir}
install -m 0755 ftd_config_gen %{buildroot}%{bin_dir}

install -m 0644 ftd.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/ftd
%{bin_dir}/fims_format_scan
%{bin_dir}/ftd_config_gen
%{systemd_dir}/ftd.service

%changelog