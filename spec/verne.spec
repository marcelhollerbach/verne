%global provider        github
%global provider_tld    com
%global project         marcelhollerbach
%global repo            verne
%global provider_prefix %{provider}.%{provider_tld}/%{project}/%{repo}
%global import_path     %{provider_prefix}
%global commit          1566a7353be2eb3dcceaf4ad12853c343b13b022
%global shortcommit     %(c=%{commit}; echo ${c:0:7})

Name:           verne
Version:        0.0.99
Release:        1%{dist}
Summary:        EFL file manager
License:        BSD-2-Clause
Group:          Productivity/Graphics/Viewers
URL:            https://%{provider_prefix}
Source0:        https://%{provider_prefix}/archive/%{commit}/%{repo}-%{shortcommit}.tar.gz
BuildRequires:  cmake check-devel libarchive-devel gcc gcc-c++ make
BuildRequires:  efl-devel enlightenment-devel
Requires:       efl
BuildRequires:  desktop-file-utils

%description
UNSTABLE: EFL file manager.

%package devel
Summary:        Development files for EFM
Requires:       verne%{?_isa} = %{version}-%{release}

%description devel
Development files for EFM.


%prep
%setup -q -n verne-%{commit}

%build
cmake . -DCMAKE_INSTALL_PREFIX:PATH=/usr/ -DCMAKE_INSTALL_LIBDIR=%{_libdir}
make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install %{?_smp_mflags}
desktop-file-validate %{buildroot}/%{_datadir}/applications/verne.desktop

%files
%defattr(-,root,root)
%doc AUTHORS COPYING README.md
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/%{name}/
%{_libdir}/emous-1/*.so
%{_libdir}/enlightenment/modules/verne/*/module.so
%{_libdir}/enlightenment/modules/verne/module.desktop
%{_libdir}/*.so
%{_datadir}/icons/hicolor/scalable/apps/verne.svg
%{_datadir}/elm_ext/elm_ext.edc.edj

%files devel
%{_includedir}/efm-1/
%{_includedir}/elm_ext-1/
%{_includedir}/emous-1/

%changelog
* Mon Jul 18 2017 v.tolstov@selfip.ru
- New snapshot
