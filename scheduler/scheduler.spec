%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    scheduler
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
FlexGen Scheduler - schedules charge and discharge events

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 scheduler %{buildroot}%{bin_dir}

install -m 0644 scheduler.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/scheduler
%{systemd_dir}/scheduler.service

%changelog