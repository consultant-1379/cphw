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

## /vobs/HW/Products/APG/MAUS/maus_dll/mausc_cnz/mausc_cxc/packages/rpm
## %define mausc_cxc_path /vobs/mas/mas_anz/maus_dll/mausc_cnz/mausc_cxc

## %syscon_cxc_path

%define syscon_cxc_path %{_cxcdir}
%define syscon_cxc_bin %{syscon_cxc_path}/bin
%define syscon_cxc_conf %{syscon_cxc_path}/conf
%define CPHWBINdir /opt/ap/cphw/bin
%define CPHWCONFdir /opt/ap/cphw/conf

%description
Installation package for SYSCON Service.

# ---------------
# preinstallation
# ---------------
%pre
##echo "This is the SYSCON Service package pre-install section"
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during installation phase"

echo "... Commands to be executed only during installation phase in pre section..."

fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during upgrade phase"
echo "... Commands to be executed only during Upgrade phase in pre section..."

rm -f %{_bindir}/syscon.sh
rm -f %{_bindir}/syscon

fi

echo "... Commands to be executed only during both phases in pre section..."

# ---------------
# installation
# ---------------
%install
##echo "This is the SYSCON Service package install section"
echo "This is the %{_name} package %{_rel} install script"

# Create necessary directories
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{CPHWBINdir}
mkdir $RPM_BUILD_ROOT%{CPHWCONFdir}

# Copy binaries & conf files to appropriate directories
cp %{syscon_cxc_bin}/syscon.sh          $RPM_BUILD_ROOT%{CPHWBINdir}/syscon.sh
cp %{syscon_cxc_bin}/syscon             $RPM_BUILD_ROOT%{CPHWBINdir}/syscon

%post
##echo "This is the SYSCON service package post-install section"
echo "This is the %{_name} package %{_rel} post-install script"

chmod +x $RPM_BUILD_ROOT%{CPHWBINdir}/syscon.sh
chmod +x $RPM_BUILD_ROOT%{CPHWBINdir}/syscon

ln -sf $RPM_BUILD_ROOT%{CPHWBINdir}/syscon.sh      $RPM_BUILD_ROOT%{_bindir}/syscon

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during installation phase"
echo "... Commands to be executed only during installation phases in post section..."
fi

# Remove config file
%preun
##echo "This is the SYCON service package pre-uninstall section"
## do nothing 
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"
echo "... Commands to be executed only during uninstallation phases in preun section..."

rm -f %{_bindir}/syscon

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
echo "... Commands to be executed only during installation phases in preun section..."

fi

%postun
##echo "This is the SYSCON service package post-uninstall section"
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
%{CPHWBINdir}/syscon.sh
%{CPHWBINdir}/syscon

%changelog

* Thu Nov 19 2015 - thanh.nguyen (at) dektech.com.au
- Update for SYSCON

