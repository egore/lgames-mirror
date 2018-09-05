/*
 * menu.h
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

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

enum {
	/* menu action ids */
	AID_NONE = 0,
	AID_FOCUSCHANGED, /* XXX used to play sound */
	AID_ENTERMENU,
	AID_LEAVEMENU,
	AID_RESUME,
	AID_HELP,
	AID_QUIT,
	AID_STARTORIGINAL,
	AID_STARTCUSTOM,
	AID_CHANGEKEY,
	AID_SOUND,
	AID_VOLUME,
	AID_APPLYAUDIO,
	AID_APPLYTHEME,
	AID_APPLYMODE
};

class Menu;

/** Basic menu item with just a caption, returns action id on click */
class MenuItem {
protected:
	Menu *parent;
	string caption;
	int x, y, w, h;
	int focus;
	int actionId;

	Font *getCurFont();
public:
	MenuItem(const string &c, int aid = AID_NONE) :
		parent(NULL), caption(c), x(0), y(0), w(1), h(1),
		focus(0), actionId(aid) {}
	virtual ~MenuItem() {}
	void setGeometry(int _x, int _y, int _w, int _h) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		_logdebug(2,"Geometry for %s: %d,%d,%d,%d\n",caption.c_str(),x,y,w,h);
	}
	void setParent(Menu *p) { parent = p; }
	bool hasPointer(int px, int py) {
		return (px >= x && px < x + w && py >= y && py < y + h);
	}
	void setFocus(int on) {
		focus = on;
	}
	virtual void update(uint ms) {}
	virtual void render();
	virtual int handleEvent(const SDL_Event &ev) {
		if (ev.type == SDL_MOUSEBUTTONDOWN)
			return actionId;
		return 0;
	}
};

/* Sub items manage destruction of submenus ... */
class MenuItemSub : public MenuItem {
	unique_ptr<Menu> submenu;
public:
	MenuItemSub(const string &c, Menu *sub)
		: MenuItem(c,AID_ENTERMENU), submenu(sub) {}
	Menu *getSubMenu() { return submenu.get(); }
};

/* ... Back items only have pointers */
class MenuItemBack : public MenuItem {
	Menu *prevMenu;
public:
	MenuItemBack(Menu *last)
		: MenuItem(_("Back"),AID_LEAVEMENU), prevMenu(last) {}
	Menu *getLastMenu() { return prevMenu; }
};

/* Range items allow modifying a referenced value. */
class MenuItemRange : public MenuItem {
protected:
	int min, max, step;
	int &val;
public:
	MenuItemRange(const string &c, int aid, int &_val,
				int _min, int _max, int _step)
		: MenuItem(c+":",aid),
		  min(_min), max(_max), step(_step), val(_val) {}
	virtual void render();
	virtual int handleEvent(const SDL_Event &ev) {
		if (ev.type == SDL_MOUSEBUTTONDOWN) {
			if (ev.button.button == SDL_BUTTON_LEFT) {
				val += step;
				if (val > max)
					val = min;
			} else if (ev.button.button == SDL_BUTTON_RIGHT) {
				val -= step;
				if (val < min)
					val = max;
			}
			return actionId;
		}
		return 0;
	}
};

class MenuItemList : public MenuItemRange {
protected:
	vector<string> options;
public:
	MenuItemList(const string &c, int aid, int &v, vector<string> &opts)
			: MenuItemRange(c,aid,v,0,opts.size()-1,1) {
		options = opts;
	}
	MenuItemList(const string &c, int aid, int &v, const char **opts, uint optNum)
			: MenuItemRange(c,aid,v,0,optNum-1,1) {
		for (uint i = 0; i < optNum; i++)
			options.push_back(opts[i]);
	}
	MenuItemList(const string &c, int aid, int &v, const string &opt1, const string &opt2)
			: MenuItemRange(c,aid,v,0,1,1) {
		options.push_back(opt1);
		options.push_back(opt2);
	}
	void setOptions(vector<string> &opts, int _cur) {
		options = opts;
		min = 0;
		max = opts.size() - 1;
		val = _cur;
	}
	virtual void render();
};

/* Value contains real integer value not the index. */
class MenuItemIntList : public MenuItemRange {
	int idx;
	int &val;
	vector<int> options;
public:
	MenuItemIntList(const string &c, int &v, const int *opts, uint optNum)
				: MenuItemRange(c,AID_NONE,idx,0,optNum-1,1), val(v) {
		idx = 0;
		for (uint i = 0; i < optNum; i++) {
			if (opts[i] == val)
				idx = i;
			options.push_back(opts[i]);
		}
		val = options[idx]; /* if not found, fallback to first value */
	}
	virtual void render();
	virtual int handleEvent(const SDL_Event &ev) {
		MenuItemRange::handleEvent(ev);
		val = options[idx];
		return AID_NONE;
	}
};

class MenuItemSwitch : public MenuItemList {
public:
	MenuItemSwitch(const string &c, int aid, int &v)
			: MenuItemList(c,aid,v,_("Off"),_("On")) {}
};

class MenuItemKey : public MenuItem {
	int &sc;
	bool waitForNewKey;
public:
	MenuItemKey(const string &c, int &_sc)
		: MenuItem(c+":",AID_CHANGEKEY), sc(_sc), waitForNewKey(false) {}
	virtual void render();
	virtual int handleEvent(const SDL_Event &ev) {
		if (ev.type == SDL_MOUSEBUTTONDOWN)
			waitForNewKey = true;
		return actionId;
	}
	void setKey(int nsc) {
		sc = nsc;
		waitForNewKey = false;
	}
	void cancelChange() { waitForNewKey = false; }
};

class Menu {
	Theme &theme;
	vector<unique_ptr<MenuItem>> items;
	MenuItem *curItem;
	static bool inputLocked;
public:
	Menu(Theme &t) : theme(t), curItem(NULL) {}
	~Menu() {}
	void add(MenuItem *item) {
		items.push_back(unique_ptr<MenuItem>(item));
		item->setParent(this);
	}
	MenuItem *getCurItem() { return curItem; }
	Font *getNormalFont() { return &theme.fMenuNormal; }
	Font *getFocusFont() { return &theme.fMenuFocus; }
	void adjust() {
		int h = items.size() * theme.menuItemHeight;
		int w = theme.menuItemWidth;
		int x = theme.menuX - w/2;
		int y = theme.menuY - h/2;
		for (uint i = 0; i < items.size(); i++) {
			MenuItemSub *sub = dynamic_cast<MenuItemSub*>(items[i].get());
			items[i]->setGeometry(x, y + i*theme.menuItemHeight,
						w, theme.menuItemHeight);
			if (sub)
				sub->getSubMenu()->adjust();
		}
	}
	void update(uint ms) {
		for (auto& i : items)
			i->update(ms);
	}
	void render() {
		for (auto& i : items)
			i->render();
	}
	int handleEvent(const SDL_Event &ev);
};

#endif /* SRC_MENU_H_ */
