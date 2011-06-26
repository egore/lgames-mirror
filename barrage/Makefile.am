SUBDIRS = src

AUTOMAKE_OPTIONS = foreign
 
RPMBUILD = rpmbuild
rpm: dist
	$(RPMBUILD) -ta $(distdir).tar.gz

desktopdir = $(datadir)/applications
desktop_DATA = barrage.desktop
icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = barrage.png 

EXTRA_DIST = BUGS README COPYING AUTHORS ChangeLog INSTALL NEWS TODO \
	$(PACKAGE).spec $(PACKAGE).spec.in barrage.desktop \
	barrage.png

