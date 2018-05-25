EXTRA_DIST = back0.jpg back1.jpg back2.jpg back3.jpg back4.jpg \
CREDITS shot.png ball.png paddle.png weapon.png \
bricks.png extras.png fsmall.otf fnormal.otf theme.ini

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/$$file;\
	done
