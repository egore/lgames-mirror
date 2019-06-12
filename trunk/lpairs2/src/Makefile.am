bin_PROGRAMS = lpairs2
lpairs2_SOURCES = sdl.cpp sdl.h tools.h tools.cpp \
	config.cpp config.h theme.cpp theme.h \
	mixer.h menu.cpp menu.h sprite.cpp sprite.h \
	main.cpp game.h game.cpp view.h view.cpp
	
SUBDIRS = themes

install-data-local:
	$(mkinstalldirs) $(DESTDIR)/$(datadir)/lpairs2

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/lpairs2

clean:
	rm -f *.o lpairs2
