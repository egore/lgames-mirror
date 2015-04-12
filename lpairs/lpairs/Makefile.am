####### kdevelop will overwrite this part!!! (begin)##########
####### kdevelop will overwrite this part!!! (end)############

DEFS = @DEFS@ @inst_flag@ @sound_flag@ @locdir_flag@

bin_PROGRAMS = lpairs
lpairs_SOURCES = main.c pairs.c dynlist.c sdl.c cfg.c menu.c sndsrv.c

#source
EXTRA_DIST = main.c pairs.c dynlist.c sdl.c cfg.c menu.c sndsrv.c \
pairs.h dynlist.h sdl.h cfg.h menu.h sndsrv.h gettext.h \
\
gfx/0.bmp gfx/1.bmp gfx/2.bmp gfx/3.bmp gfx/4.bmp gfx/5.bmp \
gfx/6.bmp gfx/7.bmp gfx/8.bmp gfx/9.bmp gfx/10.bmp gfx/11.bmp \
gfx/12.bmp gfx/13.bmp gfx/14.bmp gfx/15.bmp gfx/16.bmp gfx/17.bmp \
gfx/18.bmp gfx/19.bmp \
gfx/button.bmp gfx/back.bmp gfx/title.bmp \
gfx/f_white.bmp gfx/f_yellow.bmp gfx/f_standard.bmp \
\
sound/click.wav sound/remove.wav sound/fail.wav

install-data-local:
#directories
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/sound
#graphics
	@for file in gfx/*.bmp; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/$$file;\
	done
#sounds
	@for file in sound/*.wav; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/$$file;\
	done

uninstall-local:
	rm -rf $(DESTDIR)$(inst_dir)
