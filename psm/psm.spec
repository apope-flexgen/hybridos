%global debug_package %{nil}
%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define systemd_dir /usr/lib/systemd/system
%define dflt_dir /usr/local/dflt/psm

Summary:    psm
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
FlexGen Digital PSM

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}
install --directory %{buildroot}%{dflt_dir}

install -m 0755 psm %{buildroot}%{bin_dir}

install -m 0644 psm.service %{buildroot}%{systemd_dir}

install -m 0777 psm_dflt.json %{buildroot}%{dflt_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/psm
%{systemd_dir}/psm.service
%{dflt_dir}/psm_dflt.json

%changelog
