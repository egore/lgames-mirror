AUTOMAKE_OPTIONS = foreign
SUBDIRS = intl po libgame src 
EXTRA_DIST = gettext.h config.rpath m4/ChangeLog \
	TODO COPYING README Changelog \
	lbreakouthd.desktop.in lbreakouthd256.gif

desktopdir = $(datadir)/applications
desktop_DATA = lbreakouthd.desktop
icondir = $(datadir)/icons
icon_DATA = lbreakouthd256.gif

# We can't use static desktop file because of icon location. Is this 
# the right way to handle it? At least it works... as long as $datadir
# does not contain plus signs. :-)
lbreakouthd.desktop:
	@sed -e "s+Icon=+Icon=$(datadir)/icons/lbreakouthd256.gif+" \
	< lbreakouthd.desktop.in > lbreakouthd.desktop

ACLOCAL_AMFLAGS = -I m4
