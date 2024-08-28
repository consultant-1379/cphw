#
# spec file for configuration of package gprshm service.
#
# Copyright  (c)  2014  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#                                     giovanni.gambardella@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for GPRSHM Service.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     CPHW
BuildRoot: %_tmppath
AutoReqProv: no
Requires: APOS_OSCONFBIN

# structure of APGsubsystem CPHW subsystem
%define CPHWclusterdir %{APclusterdir}/cphw
%define CPHWdir %{APdir}/cphw

%define CPHWBINdir %{CPHWdir}/bin
%define CPHWCONFdir %{CPHWdir}/conf
%define CPHWDOCdir %{CPHWdir}/doc
%define CPHWETCdir %{CPHWdir}/etc
%define CPHWLIBdir %{CPHWdir}/lib

%define gprshm_cxc_path %{_cxcdir}
%define gprshm_cxc_bin %{gprshm_cxc_path}/bin
%define gprshm_cxc_conf %{gprshm_cxc_path}/conf
%define gprshm_cxc_script %{gprshm_cxc_path}/script

%description
Installation package for gprshm Service.

%pre
if [ $1 == 1 ] 
then
echo "This is the %{_name} package %{_rel} pre-install script during installation phase"
fi
if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} pre-install script during upgrade phase"
fi

%install
echo "This is the %{_name} package %{_rel} install script"

if [ ! -d $RPM_BUILD_ROOT%{rootdir} ]
then
    mkdir $RPM_BUILD_ROOT%{rootdir}
fi
if [ ! -d $RPM_BUILD_ROOT%{APdir} ]
then
    mkdir $RPM_BUILD_ROOT%{APdir}
fi    
if [ ! -d $RPM_BUILD_ROOT%{CPHWdir} ]
then
    mkdir $RPM_BUILD_ROOT%{CPHWdir}
fi    
if [ ! -d $RPM_BUILD_ROOT%{CPHWBINdir} ]
then
    mkdir $RPM_BUILD_ROOT%{CPHWBINdir}
fi    
if [ ! -d $RPM_BUILD_ROOT%{CPHWLIBdir} ]
then
    mkdir $RPM_BUILD_ROOT%{CPHWLIBdir}
fi
if [ ! -d $RPM_BUILD_ROOT%{CPHWETCdir} ]
then
    mkdir $RPM_BUILD_ROOT%{CPHWETCdir}
fi
if [ ! -d $RPM_BUILD_ROOT%{CPHWCONFdir} ]
then
    mkdir $RPM_BUILD_ROOT%{CPHWCONFdir}
fi

cp %{gprshm_cxc_bin}/cphw_gprshmd $RPM_BUILD_ROOT%{CPHWBINdir}/cphw_gprshmd
cp %{gprshm_cxc_bin}/t_gprshm $RPM_BUILD_ROOT%{CPHWBINdir}/t_gprshm
cp %{gprshm_cxc_script}/cphw_gprshm_clc $RPM_BUILD_ROOT%{CPHWBINdir}/cphw_gprshm_clc

%post
if [ $1 == 1 ] 
then
echo "This is the %{_name} package %{_rel} post script during installation phase"
fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} post script during upgrade phase"
fi

# For both post phases
chmod +x $RPM_BUILD_ROOT%{CPHWBINdir}/cphw_gprshmd
chmod +x $RPM_BUILD_ROOT%{CPHWBINdir}/t_gprshm
chmod +x $RPM_BUILD_ROOT%{CPHWBINdir}/cphw_gprshm_clc

ln -sf $RPM_BUILD_ROOT%{CPHWBINdir}/cphw_gprshmd $RPM_BUILD_ROOT/%{_bindir}/cphw_gprshmd
ln -sf $RPM_BUILD_ROOT%{CPHWBINdir}/t_gprshm $RPM_BUILD_ROOT/%{_bindir}/t_gprshm

%preun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} postun script during unistall phase"
rm -f %{_bindir}/cphw_gprshmd
rm -f %{_bindir}/t_gprshm
rm -f %{_bindir}/cphw_gprshm_clc
fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postun script during upgrade phase"
fi

%files
%defattr(-,root,root)
%{CPHWBINdir}/cphw_gprshmd
%{CPHWBINdir}/t_gprshm
%{CPHWBINdir}/cphw_gprshm_clc

%changelog
* Fri Dec 12 2014 - tuan.nguyen (at) dektech.com.au
- Initial implementation

 

