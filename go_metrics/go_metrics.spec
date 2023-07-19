%define debug_package %{nil} # do not build debug package

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    go_metrics
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}

%description
Go Metrics - Better than the old Metrics (ask Tony about it)

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}

install -m 0755 go_metrics %{buildroot}%{bin_dir}

install -m 0644 go_metrics@.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/go_metrics
%{systemd_dir}/go_metrics@.service

%changelog