%global __strip /bin/true # turn off binary stripping
%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define systemd_dir /usr/lib/systemd/system

Summary:    events
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, mongodb-org

%description
HybridOS Events Storage - receive events, store in database, serve to UI with filtering

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 events %{buildroot}%{bin_dir}

install -m 0644 events.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/events
%{systemd_dir}/events.service

%changelog