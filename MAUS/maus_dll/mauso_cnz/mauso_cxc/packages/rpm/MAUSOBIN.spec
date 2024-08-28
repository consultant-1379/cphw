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

# ---------------------------------------------------------
# Need to check and change if needed for every single build
# ---------------------------------------------------------

%define mauslib_rm_regex libCXC106*
%define mibs_rm_regex mibsCXC106*
#
#----------------------------------------------------------

%define CPHWclusterdir %{APclusterdir}/cphw
%define CPHWdir %{APdir}/cphw

%define CPHWBINdir %{CPHWdir}/bin
%define CPHWCONFdir %{CPHWdir}/conf
%define CPHWDOCdir %{CPHWdir}/doc
%define CPHWETCdir %{CPHWdir}/etc
%define CPHWLIBdir %{CPHWdir}/lib
%define CPHWLIB64dir %{CPHWdir}/lib64
%define CPHWTMPdir %{CPHWdir}/tmp

%define mausobin_cxc_path %{_cxcdir}

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

## Remove old so lib files and mibs files
## --------------------------------------

cd %CPHWLIB64dir 
find . -name "%{mauslib_rm_regex}" -type f | xargs -i rm {}
find . -name "%{mibs_rm_regex}" -type f | xargs -i rm {}

fi 

%install

mkdir -p $RPM_BUILD_ROOT%CPHWBINdir
mkdir -p $RPM_BUILD_ROOT%CPHWCONFdir
mkdir -p $RPM_BUILD_ROOT%CPHWDOCdir
mkdir -p $RPM_BUILD_ROOT%CPHWETCdir
mkdir -p $RPM_BUILD_ROOT%CPHWLIBdir
mkdir -p $RPM_BUILD_ROOT%CPHWLIB64dir
mkdir -p $RPM_BUILD_ROOT%CPHWTMPdir

ls -l %mausobin_cxc_path/bin/lib_ext
#cp -f %mausobin_cxc_path/bin/lib_ext/* $RPM_BUILD_ROOT%CPHWLIB64dir/
for file in $(ls  %mausobin_cxc_path/bin/lib_ext/)
do
cp -f %mausobin_cxc_path/bin/lib_ext/$file $RPM_BUILD_ROOT%CPHWLIB64dir/
done

%post
if [ $1 == 1 ] 
then
echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"

fi

## Apply chmod +x for all libCXC106 file
find %{CPHWLIB64dir} -name '*.txt' | grep libCXC106 | awk '{print substr($0, 1, length($0)-4)}' | xargs chmod +x

%preun
if [ $1 == 0 ] 
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"
fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"

cd %CPHWLIB64dir 
find . -name "%{mauslib_rm_regex}" -type f | xargs -i rm {}
find . -name "%{mibs_rm_regex}" -type f | xargs -i rm {}

fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%CPHWLIB64dir

