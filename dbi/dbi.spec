%global __strip /bin/true # turn off binary stripping
%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system
%define opt_dir /opt/flexgen/
%define node_dir /usr/lib/node_modules
%define go_dir /usr/lib/golang/src

Summary:    dbi
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, mongodb-org, mongodb-org-mongos, mongodb-org-server, mongodb-org-shell, mongodb-org-tools

%description
General database interface from FIMS to MongoDB or other backends

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}
install --directory %{buildroot}%{opt_dir}

install -m 0755 dbi %{buildroot}%{bin_dir}

install -m 0644 dbi.service %{buildroot}%{systemd_dir}
install -m 0644 dbi.repo %{buildroot}%{opt_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/dbi
%{systemd_dir}/dbi.service
%{opt_dir}/dbi.repo

%changelog
