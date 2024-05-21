%global __strip /bin/true # turn off binary stripping
%define debug_package %{nil} # preserve debug information
%define _build_id_links none

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define systemd_dir /usr/lib/systemd/system

Summary:    metrics
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
Metrics computation from FIMS pubs/sets

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 metrics %{buildroot}%{bin_dir}

install -m 0644 metrics@.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/metrics
%{systemd_dir}/metrics@.service

%changelog