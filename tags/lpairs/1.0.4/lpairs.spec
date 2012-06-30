Name:           lpairs
Version:        1.0
Release:        1%{?dist}
Summary:        Classical memory game with cards

Group:          Amusements/Games
License:        GPL
URL:            http://lgames.sourceforge.net/index.php?project=LPairs
Source0:        http://lgames.sourceforge.net/download.php?project=LPairs&url=SOURCEFORGE/lgames/lpairs-1.0.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

#BuildRequires:  
#Requires:       

%description
LPairs is a classical memory game. This means you have to 
find pairs of identical cards which will then be removed. 
Your time and tries needed will be counted but there is no
highscore chart or limit to this.

%prep
%setup -q

# create .desktop file for GNOME, KDE, etc. menus
cat > %name.desktop <<EOF
[Desktop Entry]
Name=LPairs
Comment=The classic memory game with cards
Exec=/usr/bin/lpairs
Type=Application
Terminal=false
Categories=Application;Game;Education;
Encoding=UTF-8
X-Desktop-File-Install-Version=0.9
EOF




%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/%{_datadir}/applications
cp %{name}.desktop $RPM_BUILD_ROOT/%{_datadir}/applications


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog  README TODO 
%{_bindir}/%{name}
%{_datadir}/games/%{name}
%{_datadir}/applications/%{name}.desktop


%changelog
