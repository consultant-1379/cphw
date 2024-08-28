#
# spec file for configuration of package MAUS1 service
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#                                     giovanni.gambardella@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for MAUS1 Service.
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

## %maus1_cxc_path/packages/rpm
##%define maus1_cxc_path /vobs/mas/mas_anz/maus_dll/maus_cnz/maus1_cxc

%define maus1_cxc_path %{_cxcdir}
%define maus1_cxc_bin %{maus1_cxc_path}/bin
%define CPHWBINdir /opt/ap/cphw/bin
%define maus1_lib_path %{maus1_cxc_bin}/lib_ext
%define CPHWLIB64dir /opt/ap/cphw/lib64

%description
Installation package for MAUS1 Service.

# ----------------
# preinstallation
# ----------------
%pre
##echo "This is the MAUS1 Service package pre-install section"
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during installation phase"

echo "... Commands to be executed only during installation phase in pre section..."

fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during upgrade phase"
echo "... Commands to be executed only during Upgrade phase in pre section..."

rm -f %{_bindir}/cphw_maus1d
rm -f %{_bindir}/cphw_maus1_clc
rm -f %{_bindir}/mauls
rm -f %{_bindir}/mauls.sh
##rm -f %{_lib64dir}/libcphw_mauslibh.so
##rm -f %{_lib64dir}/libcphw_mauslibh.so.1

rm -f %{_bindir}/mauset
rm -f %{_bindir}/maulock
rm -f %{_bindir}/mauunlock
rm -f %{_bindir}/mauadmls

rm -f %{_bindir}/maulock.sh
rm -f %{_bindir}/mauunlock.sh
rm -f %{_bindir}/mauadmls.sh
rm -f %{_bindir}/mauset.sh

fi

echo "... Commands to be executed only during both phases in pre section..."

# ---------------
# installation
# ---------------
%install
##echo "This is the MAUS1 Service package install section"
echo "This is the %{_name} package %{_rel} install script"

# Create necessary directories
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{CPHWBINdir}
mkdir $RPM_BUILD_ROOT/%{CPHWLIB64dir}

# Copy binaries & conf files to appropriate directories
cp %{maus1_cxc_bin}/cphw_maus1d     $RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_maus1d
cp %{maus1_cxc_bin}/mauls           $RPM_BUILD_ROOT/%{CPHWBINdir}/mauls
cp %{maus1_cxc_bin}/cphw_maus1_clc  $RPM_BUILD_ROOT/%{CPHWBINdir}/cphw_maus1_clc
cp %{maus1_cxc_bin}/mauls.sh        $RPM_BUILD_ROOT/%{CPHWBINdir}/mauls.sh

# Black market stuff
# ------------------
##cp %{maus1_cxc_bin}/maulock         $RPM_BUILD_ROOT/%{CPHWBINdir}/maulock
##cp %{maus1_cxc_bin}/mauunlock       $RPM_BUILD_ROOT/%{CPHWBINdir}/mauunlock
##cp %{maus1_cxc_bin}/mauadmls        $RPM_BUILD_ROOT/%{CPHWBINdir}/mauadmls

##cp %{maus1_cxc_bin}/maulock.sh      $RPM_BUILD_ROOT/%{CPHWBINdir}/maulock.sh
##cp %{maus1_cxc_bin}/mauunlock.sh    $RPM_BUILD_ROOT/%{CPHWBINdir}/mauunlock.sh
##cp %{maus1_cxc_bin}/mauadmls.sh     $RPM_BUILD_ROOT/%{CPHWBINdir}/mauadmls.sh

##cp %{maus1_lib_path}/libcphw_mauslibh.so.1.1.0 $RPM_BUILD_ROOT/%{CPHWLIB64dir}/libcphw_mauslibh.so.1.1.0

%post
##echo "This is the MAUS1 service package post-install section"
echo "This is the %{_name} package %{_rel} post-install script"

chmod +x %{CPHWBINdir}/cphw_maus1d
chmod +x %{CPHWBINdir}/mauls
chmod +x %{CPHWBINdir}/cphw_maus1_clc
chmod +x %{CPHWBINdir}/mauls.sh

##chmod +x %{CPHWLIB64dir}/libcphw_mauslibh.so.1.1.0

# Black market stuff
# ------------------
##chmod +x %{CPHWBINdir}/maulock
##chmod +x %{CPHWBINdir}/mauunlock
##chmod +x %{CPHWBINdir}/mauadmls

##chmod +x %{CPHWBINdir}/maulock.sh
##chmod +x %{CPHWBINdir}/mauunlock.sh
c##hmod +x %{CPHWBINdir}/mauadmls.sh

ln -sf %{CPHWBINdir}/cphw_maus1d    %{_bindir}/cphw_maus1d
ln -sf %{CPHWBINdir}/mauls.sh       %{_bindir}/mauls

##ln -sf %{CPHWLIB64dir}/libcphw_mauslibh.so.1.1.0  %{_lib64dir}/libcphw_mauslibh.so.1
##ln -sf %{_lib64dir}/libcphw_mauslibh.so.1  %{_lib64dir}/libcphw_mauslibh.so

##pkill cphw_maus1d

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during installation phase"
echo "... Commands to be executed only during installation phases in post section..."
fi

%preun
##echo "This is the MAUS1 service package pre-uninstall section"
## do nothing 
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"
echo "... Commands to be executed only during uninstallation phases in preun section..."

rm -f %{_bindir}/cphw_maus1d
rm -f %{_bindir}/mauls
rm -f %{_bindir}/maulock
rm -f %{_bindir}/mauunlock
rm -f %{_bindir}/mauadmls
rm -f %{_bindir}/mauset

##rm -f %{_lib64dir}/libcphw_mauslibh.so
##rm -f %{_lib64dir}/libcphw_mauslibh.so.1

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
echo "... Commands to be executed only during installation phases in preun section..."

fi

%postun
##echo "This is the MAUS1 service package post-uninstall section"
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
%{CPHWBINdir}/cphw_maus1d
%{CPHWBINdir}/cphw_maus1_clc
%{CPHWBINdir}/mauls
%{CPHWBINdir}/mauls.sh
##%{CPHWBINdir}/maulock.sh
##%{CPHWBINdir}/mauunlock.sh
##%{CPHWBINdir}/mauadmls.sh
##%{CPHWBINdir}/maulock
##%{CPHWBINdir}/mauunlock
##%{CPHWBINdir}/mauadmls

%changelog
* Mon Aug 11 2014 - thanh.nguyen (at) dektech.com.au
- First release 

