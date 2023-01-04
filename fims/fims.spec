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

Summary:    fims
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Provides:   libfims.so()(64bit)
Requires:   cjson

%description
FlexGen Internal Messaging Service

%package devel
Summary:fims-devel
Group:fims

%description devel
FlexGen Internal Messaging Service Development Files

%prep
%setup -q -n %{source}

%build

%install

install --directory %{buildroot}%{bin_dir}
install -m 0755 fims_server %{buildroot}%{bin_dir}
install -m 0755 fims_listen %{buildroot}%{bin_dir}
install -m 0755 fims_send %{buildroot}%{bin_dir}
install -m 0755 fims_echo %{buildroot}%{bin_dir}
install -m 0755 fims_trigger %{buildroot}%{bin_dir}

install --directory %{buildroot}%{lib_dir}
install -m 0755 libfims.so %{buildroot}%{lib_dir}
install -m 0755 fims.node %{buildroot}%{lib_dir}

install --directory %{buildroot}%{systemd_dir}
install -m 0644 fims.service %{buildroot}%{systemd_dir}

install --directory %{buildroot}%{opt_dir}
install -m 0644 fims.repo %{buildroot}%{opt_dir}

# devel
install --directory %{buildroot}%{include_dir}/fims
install -m 0755 fims.h %{buildroot}%{include_dir}/fims
install -m 0755 fps_utils.h %{buildroot}%{include_dir}/fims
install -m 0755 libfims.h %{buildroot}%{include_dir}/fims
install -m 0755 defer.hpp %{buildroot}%{include_dir}/fims

install --directory %{buildroot}%{go_dir}/fims
install -m 0755 fims.go %{buildroot}%{go_dir}/fims
install -m 0755 msg.go %{buildroot}%{go_dir}/fims
install -m 0755 verification.go %{buildroot}%{go_dir}/fims

install --directory %{buildroot}%{node_dir}/fims
install -m 0755 fimsListener.js %{buildroot}%{node_dir}/fims
install -m 0755 package.json %{buildroot}%{node_dir}/fims
install -m 0664 node_modules.tar.gz %{buildroot}%{node_dir}/fims

%post devel

echo "unpacking node_modules"
tar -xzf %{node_dir}/fims/node_modules.tar.gz -C %{node_dir}/fims
rm %{node_dir}/fims/node_modules.tar.gz

%clean
rm -rf %{buildroot}

%files
%{bin_dir}/fims_server
%{bin_dir}/fims_listen
%{bin_dir}/fims_send
%{bin_dir}/fims_echo
%{bin_dir}/fims_trigger
%{lib_dir}/libfims.so
%{lib_dir}/fims.node
%{systemd_dir}/fims.service
%{opt_dir}/fims.repo

%files devel
%{go_dir}/fims
%{include_dir}/fims
%attr(4775, root, wheel) %{node_dir}/fims

%changelog
