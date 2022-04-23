EXTRA_DIST = grass.jpg metal.jpg water.jpg wood.jpg

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard/backgrounds
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/backgrounds/$$file;\
	done
