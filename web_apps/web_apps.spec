%global __strip /bin/true # turn off binary stripping
%define debug_package %{nil} # preserve debug information

%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define log_dir /var/log
%define systemd_dir /usr/lib/systemd/system

Summary:    web_apps
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, mongodb-org

%description
FlexGen HybridOS Web Apps

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{bin_dir}/web_ui
install --directory %{buildroot}%{log_dir}/web_server
install --directory %{buildroot}%{systemd_dir}

install -m 0755 web_server %{buildroot}%{bin_dir}
install -m 0644 web_server.service %{buildroot}%{systemd_dir}
install -m 0664 build.tar.gz %{buildroot}%{bin_dir}/web_ui

%post
echo "changing ownership of web_server binary"
/bin/chown -R hybridos:hybridos %{log_dir}/web_server

echo "unpacking web_ui build directory"
tar -xzvf %{bin_dir}/web_ui/build.tar.gz -C %{bin_dir}/web_ui
rm %{bin_dir}/web_ui/build.tar.gz

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/web_server
%{bin_dir}/web_ui
%{log_dir}/web_server
%{systemd_dir}/web_server.service

%changelog
