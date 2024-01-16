%define source %{_name}-%{_version}-%{_release}
%define bin_dir /usr/local/bin
%define lib_dir /usr/local/lib
%define include_dir /usr/local/include
%define systemd_dir /usr/lib/systemd/system

Summary:    ess controller
License:    FlexGen Power Systems
Name:       %{_name}
Version:    %{_version}
Release:    %{_release}
Source:     %{source}.tar.gz
BuildRoot:  %{_topdir}
Requires:   fims, cjson

%description
Ess Controller System

%prep
%setup -q -n %{source}

%build

%install
install --directory %{buildroot}%{bin_dir}
install --directory %{buildroot}%{systemd_dir}
install --directory %{buildroot}%{lib_dir}

install -m 0755 ess_controller %{buildroot}%{bin_dir}
install -m 0755 gpio_controller %{buildroot}%{bin_dir}
install -m 0755 libess.so %{buildroot}%{lib_dir}
install -m 0755 libessfunc.so %{buildroot}%{lib_dir}

install -m 0644 ess_controller@.service %{buildroot}%{systemd_dir}
install -m 0644 gpio_controller.service %{buildroot}%{systemd_dir}
install -m 0644 modprobe_i2c.service %{buildroot}%{systemd_dir}

%clean
rm -rf %{buildroot}

%files
%{lib_dir}/libess.so
%{lib_dir}/libessfunc.so
%{bin_dir}/ess_controller
%{bin_dir}/gpio_controller
%{systemd_dir}/ess_controller@.service
%{systemd_dir}/gpio_controller.service
%{systemd_dir}/modprobe_i2c.service

%changelog