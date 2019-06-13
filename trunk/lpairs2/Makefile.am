AUTOMAKE_OPTIONS = foreign
SUBDIRS = po src 
EXTRA_DIST = gettext.h config.rpath \
	TODO COPYING README INSTALL ChangeLog \
	lpairs2.desktop.in lpairs2.png

desktopdir = $(datadir)/applications
desktop_DATA = lpairs2.desktop
icondir = $(datadir)/icons
icon_DATA = lpairs2.png

# We can't use static desktop file because of icon location. Is this 
# the right way to handle it? At least it works... as long as $datadir
# does not contain plus signs. :-)
lpairs2.desktop:
	@sed -e "s+Icon=+Icon=$(datadir)/icons/lpairs2.png+" \
	< lpairs2.desktop.in > lpairs2.desktop

ACLOCAL_AMFLAGS = -I m4
