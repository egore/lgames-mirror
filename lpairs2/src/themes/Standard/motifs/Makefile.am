EXTRA_DIST = \
Badger.jpg \
Brown_Bear.jpg \
Cat.jpg \
Cobra.jpg \
Colibri.jpg \
Cow.jpg \
Deer.jpg \
Dolphin.jpg \
Eagle.jpg \
Elephant.jpg \
Fox.jpg \
German_Sheperd.jpg \
Ginuea_Pig.jpg \
Giraffe.jpg \
Goat.jpg \
Gorilla.jpg \
Hippo.jpg \
Horse.jpg \
Koala.jpg \
Lama.jpg \
Lion.jpg \
Orca.jpg \
Owl.jpg \
Parrot.jpg \
Penguin.jpg \
Polar_Bear.jpg \
Rabbit.jpg \
Rhino.jpg \
Seagull.jpg \
Shark.jpg \
Sheep.jpg \
Squirrel.jpg \
Swan.jpg \
Tiger.jpg \
Wolf.jpg \
Zebra.jpg

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datdir)/themes/Standard/motifs
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(datdir)/themes/Standard/motifs/$$file;\
	done
