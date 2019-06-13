EXTRA_DIST =     p_colibri.jpg        p_hippo.jpg    p_seagull.jpg \
c_back.png       p_cow.jpg            p_horse.jpg    p_shark.jpg \
c_focus.png      p_deer.jpg           p_icebear.jpg  p_sheep.jpg \
c_front.png      p_dolphin.jpg        p_koala.jpg    p_squirrel.jpg \
CREDITS          p_eagle.jpg          p_lama.jpg     p_swan.jpg \
f_bold.otf       p_elephant.jpg       p_lion.jpg     p_tiger.jpg \
f_normal.otf     p_fox.jpg            p_orca.jpg     p_wolf.jpg \
                 p_germansheperd.jpg  p_owl.jpg      p_zebra.jpg \
p_badger.jpg     p_ginueapig.jpg      p_parrot.jpg   s_click.wav \
p_cobra.jpg      p_giraffe.jpg        p_penguin.jpg  s_fail.wav \
p_brownbear.jpg  p_goat.jpg           p_rabbit.jpg   SIL-OFL.txt \
p_cat.jpg        p_gorilla.jpg        p_rhino.jpg    s_remove.wav \
s_menumotion.wav s_menuclick.wav      menu.png \
b_0.jpg b_1.jpg b_2.jpg b_3.jpg

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/$$file;\
	done
