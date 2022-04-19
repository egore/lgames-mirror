EXTRA_DIST = \
c_back.png c_focus.png c_front.png CREDITS
f_bold.otf f_normal.otf SIL-OFL.txt \
menu.png \
s_fail.wav s_menuclick.wav s_menumotion.wav s_remove.wav

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/$$file;\
	done
