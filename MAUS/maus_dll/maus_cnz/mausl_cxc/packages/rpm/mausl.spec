# spec file for configuration of package apache
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for %{_name}
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     CPHW
BuildRoot: %_tmppath
AutoReqProv: no
Requires: APOS_OSCONFBIN

#----------------------------------------------------------

%define CPHWclusterdir %{APclusterdir}/cphw
%define CPHWdir %{APdir}/cphw

%define CPHWLIB64dir %{CPHWdir}/lib64
%define mausl_cxc_path %{_cxcdir}
%define mausl_cxc_bin %{mausl_cxc_path}/bin
%define mausl_lib_path %{mausl_cxc_bin}/lib_ext
%define mauslib libcphw_mauslibh.so.1.1.2

%description
Installation package for %{_name}  %{_prNr} %{_rel}

%pre
if [ $1 == 1 ] 
then
echo "This is the %{_name} package %{_rel} pre-install script during installation phase"
fi
if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} pre-install script during upgrade phase"

## Remove old so lib files 
## -----------------------

rm -f %{_lib64dir}/libcphw_mauslibh.so
rm -f %{_lib64dir}/libcphw_mauslibh.so.1

fi 

%install

mkdir -p $RPM_BUILD_ROOT%CPHWLIB64dir

echo "----> %mausl_lib_path"

ls -l %{mausl_lib_path}
cp -f %{mausl_lib_path}/%mauslib $RPM_BUILD_ROOT/%{CPHWLIB64dir}/

%post
if [ $1 == 1 ] 
then
echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"

fi

## Apply chmod +x for lib file
chmod +x %{CPHWLIB64dir}/%mauslib
ln -sf %{CPHWLIB64dir}/%mauslib  %{_lib64dir}/libcphw_mauslibh.so.1
ln -sf %{_lib64dir}/libcphw_mauslibh.so.1  %{_lib64dir}/libcphw_mauslibh.so

%preun
if [ $1 == 0 ] 
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"

rm -f %{_lib64dir}/libcphw_mauslibh.so
rm -f %{_lib64dir}/libcphw_mauslibh.so.1

fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"
fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%{CPHWLIB64dir}/%{mauslib}

