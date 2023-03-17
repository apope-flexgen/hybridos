%global __strip /bin/true # turn off binary stripping
%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define log_dir /var/log
%define systemd_dir /usr/lib/systemd/system

Summary:    web_server
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
FlexGen HybridOS Web Server

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{log_dir}/web_server
install --directory %{buildroot}%{systemd_dir}

install -m 0755 web_server %{buildroot}%{bin_dir}
install -m 0644 web_server.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%post
/bin/chown -R hybridos:hybridos %{log_dir}/web_server

%files
%{bin_dir}/web_server
%{log_dir}/web_server
%{systemd_dir}/web_server.service

%changelog