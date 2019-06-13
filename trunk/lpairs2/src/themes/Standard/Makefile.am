EXTRA_DIST = \
b_0.jpg                              p_goat.jpg     p_shark.jpg \
b_1.jpg         p_badger.jpg         p_gorilla.jpg  p_sheep.jpg \
b_2.jpg         p_brownbear.jpg      p_hippo.jpg    p_squirrel.jpg \
b_3.jpg         p_cat.jpg            p_horse.jpg    p_swan.jpg \
c_back.png      p_cobra.jpg          p_icebear.jpg  p_tiger.jpg \
c_focus.png     p_colibri.jpg        p_koala.jpg    p_wolf.jpg \
c_front.png     p_cow.jpg            p_lama.jpg     p_zebra.jpg \
CREDITS         p_deer.jpg           p_lion.jpg     s_click.wav \
f_bold.otf      p_dolphin.jpg        p_orca.jpg     s_fail.wav \
f_normal.otf    p_eagle.jpg          p_owl.jpg      SIL-OFL.txt \
                p_elephant.jpg       p_parrot.jpg   s_menuclick.wav \
                p_fox.jpg            p_penguin.jpg  s_menumotion.wav \
                p_germansheperd.jpg  p_rabbit.jpg   s_remove.wav \
menumotion.wav  p_ginueapig.jpg      p_rhino.jpg \
menu.png        p_giraffe.jpg        p_seagull.jpg

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/$$file;\
	done
