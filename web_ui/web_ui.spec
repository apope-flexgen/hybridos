%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define systemd_dir /usr/lib/systemd/system

Summary:    web_ui
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   web_server

%description
FlexGen HybridOS Web Server

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}/web_ui

install -m 0664 build.tar.gz %{buildroot}%{bin_dir}/web_ui

%post
echo "Unpacking web_ui build directory"
tar -xzvf %{bin_dir}/web_ui/build.tar.gz -C %{bin_dir}/web_ui
rm %{bin_dir}/web_ui/build.tar.gz

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/web_ui

%changelog