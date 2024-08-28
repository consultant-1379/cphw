#
# spec file for configuration of package MAUSC service
#
# Copyright  (c)  2015  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#                                     giovanni.gambardella@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for MAUSC Service.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Application
BuildRoot: %_tmppath

Requires: APOS_OSCONFBIN
# This will make rpm not find the dependencies 
# TODO: should ignore dependecies?
Autoreq: 0

## %mausc_cxc_path/packages/rpm
## %define mausc_cxc_path /vobs/mas/mas_anz/maus_dll/mausc_cnz/mausc_cxc

%define mausc_cxc_path %{_cxcdir}
%define mausc_cxc_bin %{mausc_cxc_path}/bin
%define mausc_cxc_conf %{mausc_cxc_path}/conf
%define CPHWBINdir /opt/ap/cphw/bin

%description
Installation package for MAUSC Service.

# ---------------
# preinstallation
# ---------------
%pre
##echo "This is the MAUSC Service package pre-install section"
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during installation phase"

echo "... Commands to be executed only during installation phase in pre section..."

fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during upgrade phase"
echo "... Commands to be executed only during Upgrade phase in pre section..."

rm -f %{_bindir}/cphw_mauscd
rm -f %{_bindir}/cphw_mausc_clc

fi

echo "... Commands to be executed only during both phases in pre section..."

# ---------------
# installation
# ---------------
%install
##echo "This is the MAUSC Service package install section"
echo "This is the %{_name} package %{_rel} install script"

# Create necessary directories
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{CPHWBINdir}
mkdir $RPM_BUILD_ROOT/%{CPHWCONFdir}

# Copy binaries & conf files to appropriate directories
cp %{mausc_cxc_bin}/cphw_mauscd  	$RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_mauscd
cp %{mausc_cxc_bin}/cphw_mausc_clc	$RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_mausc_clc

%post
##echo "This is the MAUSC service package post-install section"
echo "This is the %{_name} package %{_rel} post-install script"

chmod +x %{CPHWBINdir}/cphw_mauscd
chmod +x %{CPHWBINdir}/cphw_mausc_clc

ln -sf %{CPHWBINdir}/cphw_mauscd %{_bindir}/cphw_mauscd

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during installation phase"
echo "... Commands to be executed only during installation phases in post section..."
fi

# Remove config file
%preun
##echo "This is the MAUSC service package pre-uninstall section"
## do nothing 
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"
echo "... Commands to be executed only during uninstallation phases in preun section..."

rm -f %{_bindir}/cphw_mauscd

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
echo "... Commands to be executed only during installation phases in preun section..."

fi

%postun
##echo "This is the MAUSC service package post-uninstall section"
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} postun script during unistall phase"
echo "... Commands to be executed only during uninstallation phases in postun section..."
fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postun script during upgrade phase"
echo "... Commands to be executed only during upgrade phases in postun section..."
fi

%files
%defattr(-,root,root)
%{CPHWBINdir}/cphw_mauscd
%{CPHWBINdir}/cphw_mausc_clc

%changelog

* Mon Feb 03 2015 - thanh.nguyen (at) dektech.com.au
- Update for SW MAU CpMauM MOM

