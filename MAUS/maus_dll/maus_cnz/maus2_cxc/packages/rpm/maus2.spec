#
# spec file for configuration of package BUSRV service
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#                                     giovanni.gambardella@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for MAUS2 Service.
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

## %maus2_cxc_path
## %define maus2_cxc_path /vobs/mas/mas_anz/maus_dll/maus_cnz/maus2_cxc

%define maus2_cxc_path %{_cxcdir}
%define maus2_cxc_bin %{maus2_cxc_path}/bin
%define CPHWBINdir /opt/ap/cphw/bin

%description
Installation package for MAUS2 Service.

# ---------------
# preinstallation
# ---------------
%pre
##echo "This is the MAUS2 Service package pre-install section"
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during installation phase"

echo "... Commands to be executed only during installation phase in pre section..."

fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during upgrade phase"
echo "... Commands to be executed only during Upgrade phase in pre section..."

rm -f %{_bindir}/cphw_maus2d
rm -f %{_bindir}/cphw_maus2_clc

fi

echo "... Commands to be executed only during both phases in pre section..."

# ---------------
# installation
# ---------------
%install
##echo "This is the MAUS2 Service package install section"
echo "This is the %{_name} package %{_rel} install script"

# Create necessary directories
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{CPHWBINdir}

# Copy binaries & conf files to appropriate directories
cp %{maus2_cxc_bin}/cphw_maus2d     $RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_maus2d
cp %{maus2_cxc_bin}/cphw_maus2_clc  $RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_maus2_clc

%post
##echo "This is the MAUS2 service package post-install section"
echo "This is the %{_name} package %{_rel} post-install script"

chmod +x %{CPHWBINdir}/cphw_maus2d
chmod +x %{CPHWBINdir}/cphw_maus2_clc

ln -sf %{CPHWBINdir}/cphw_maus2d    %{_bindir}/cphw_maus2d

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during installation phase"
echo "... Commands to be executed only during installation phases in post section..."
fi

%preun
##echo "This is the BUSRV service package pre-uninstall section"
## do nothing 
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"
echo "... Commands to be executed only during uninstallation phases in preun section..."

rm -f %{_bindir}/cphw_maus2d

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
echo "... Commands to be executed only during installation phases in preun section..."

fi

%postun
##echo "This is the MAUS2 service package post-uninstall section"
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
%{CPHWBINdir}/cphw_maus2d
%{CPHWBINdir}/cphw_maus2_clc

%changelog
* Mon Aug 11 2014 - thanh.nguyen (at) dektech.com.au
- First release 

