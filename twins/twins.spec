%global debug_package %{nil}
%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define systemd_dir /usr/lib/systemd/system
%define dflt_dir /usr/local/dflt/twins

Summary:    twins
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims

%description
FlexGen Digital Twins

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}
install --directory %{buildroot}%{dflt_dir}

install -m 0755 twins %{buildroot}%{bin_dir}

install -m 0644 twins.service %{buildroot}%{systemd_dir}

install -m 0777 twins_dflt.json %{buildroot}%{dflt_dir}

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/twins
%{systemd_dir}/twins.service
%{dflt_dir}/twins_dflt.json

%changelog
