%define bin_dir /usr/local/bin
%define systemd_dir /usr/lib/systemd/system
%define version_dir /usr/etc/flexgen

%define source %{_name}-%{_version}-%{_release}

Summary:    %{_name}
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}

Requires: %{_reqs}

%description
Install HybridOS %{_name} Product

%prep
%setup -q -n %{source}

%install
install --directory %{buildroot}%{version_dir}
install -m 0444 version.txt %{buildroot}%{version_dir}

%files
%{version_dir}/version.txt

%changelog
