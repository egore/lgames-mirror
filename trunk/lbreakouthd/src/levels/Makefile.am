# Use ls | grep -v Makefile | sed -e "s/$/ \\\\/" for update
EXTRA_DIST = LBreakout1 LBreakout2

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/levels
	@for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/levels/$$file; \
	done

