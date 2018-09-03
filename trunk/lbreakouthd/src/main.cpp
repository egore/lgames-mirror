/*
 * main.cpp
 * (C) 2018 by Michael Speck
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

using namespace std;

#include "sdl.h"
#include "tools.h"
#include "hiscores.h"
#include "clientgame.h"
#include "mixer.h"
#include "theme.h"
#include "sprite.h"
#include "menu.h"
#include "view.h"

/* for parsing arguments old-school because no easy way in C++11 */
#include <unistd.h>
extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
	/* i18n */
#ifdef ENABLE_NLS
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif

	Config config;

	/* parse command line options
	 * TODO gimme a nice menu, boi */
	string setname = "LBreakout2";
	string themename = "Standard";
	string str;
	int pnum = 0;
	string pnames[MAXCONFIGPLAYERNAMES];
	size_t pos;
	int opt;
	while ((opt = getopt(argc, argv, "fp:w:t:h")) != -1) {
		switch (opt) {
		case 'f':
			config.fullscreen = 1;
			break;
		case 'w':
			config.fullscreen = 0;
			config.resolution = atoi(optarg);
			break;
		case 'p':
			str = optarg;
			while ((pos = str.find(",")) != string::npos && pnum < MAXCONFIGPLAYERNAMES-1) {
				config.player_names[pnum++] = str.substr(0,pos);
				str = str.substr(pos+1);
			}
			config.player_names[pnum++] = str;
			config.player_count = pnum;
			break;
		case 't':
			themename = string(optarg);
			break;
		case 'h':
		default:
			_loginfo("Usage: %s [-w resolution] [-f] [-p player1,...] [-t theme] levelset\n", argv[0]);
			_loginfo("  -w window resolution in vertical pixels (fixed to 16:9 ratio)\n");
			_loginfo("  -f fullscreen on current screen resolution\n");
			_loginfo("  -p up to four comma separated player names\n");
			_loginfo("  -t theme to be used (LBreakout2 themes ok, too)\n");
			_loginfo("Options are saved to config file, only needed to set new values.\n");
			_loginfo("If levelset is missing default LBreakout2 is used.\n");
			_loginfo("Themes/sets may start with ~ to load from ~/.lbreakouthd\n");
			_loginfo("  (only if installed with make install).\n");
			_loginfo("Example: %s -w 720 -p Mike,Tom LBreakout1\n",argv[0]);
			exit(1);
		}
	}
	if (optind < argc)
		setname = string(argv[optind]);

	ClientGame cgame(config);
	View view(config, cgame);
	if (themename != "Standard")
		view.loadTheme(themename);

	srand(time(NULL));
	view.runMenu();
	//cgame.init(setname);
	//view.run();

	return 0;
}
