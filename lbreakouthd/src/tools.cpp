/*
 * tools.cpp
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

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tools.h"

string trimString(const string& str)
{
	size_t start = 0;
	while (str[start] <= 32 && start < str.length())
		start++;
	size_t len = 0;
	while (str[start + len] > 32 && start + len < str.length())
		len++;
	return str.substr(start,len);
}

/** Parse text file in format:
 * entry=...
 * set {
 * 	entry=...
 * 	...
 * }
 * ...
 */
FileParser::FileParser(const string&  fname)
{
	string line;
	ifstream ifs(fname);
	size_t pos;

	if (!ifs.is_open()) {
		_logerr("Could not open %s\n",fname.c_str());
		return;
	}
	_logdebug(1,"Parsing file %s\n",fname.c_str());

	prefix="";
	while (getline(ifs,line)) {
		if ((pos = line.find('=')) != string::npos) {
			/* plain entry, add to list */
			string key = prefix + trimString(line.substr(0, pos));
			string val = trimString(line.substr(pos+1));
			ParserEntry pe = {key,val};
			entries.push_back(pe);
			_logdebug(2,"  %s=%s\n",key.c_str(),val.c_str());
		} else if ((pos = line.find('{')) != string::npos) {
			/* new subset, adjust prefix */
			prefix += trimString(line.substr(0,pos)) + ".";
		} else if ((pos = line.find('}')) != string::npos) {
			/* subset done, reduce prefix */
			prefix = prefix.substr(0,prefix.length()-1);
			if ((pos = prefix.find('.')) == string::npos)
				prefix = "";
			else
				prefix = prefix.substr(0,pos) + ".";
		}
	}
	ifs.close();
}

/** Get value as string, double or int. Return 1 on success, 0 otherwise.
 * Leave value unchanged if not found. */
int FileParser::get(const string& k, string &v)
{
	for (auto& e : entries)
		if (e.key == k) {
			v = e.value;
			return 1;
		}
	_logdebug(2,"Entry %s not found\n",k.c_str());
	return 0;
}
int FileParser::get(const string& k, int &v)
{
	string str;
	int ret = get(k,str);
	if (ret)
		v = stoi(str);
	return ret;
}
int FileParser::get(const string& k, uint &v)
{
	string str;
	int ret = get(k,str);
	if (ret) {
		int i = stoi(str);
		if (i < 0)
			_logerr("%s value %d is not uint\n",k.c_str(),i);
		v = i;
	}
	return ret;
}
int FileParser::get(const string& k, uint8_t &v)
{
	string str;
	int ret = get(k,str);
	if (ret) {
		int i = stoi(str);
		if (i < 0 || i > 255)
			_logerr("%s value %d is not uint8_t!\n",k.c_str(),i);
		v = i;
	}
	return ret;
}
int FileParser::get(const string& k, double &v)
{
	string str;
	int ret = get(k,str);
	if (ret)
		v = stod(str);
	return ret;
}

bool dirExists(const string& name) {
	struct stat info;
	return (stat(name.c_str(), &info) == 0);
}

bool makeDir(const string& name) {
	return mkdir(name.c_str(), S_IRWXU) == 0;
}

bool fileExists(const string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	return false;
}

/** Not the nicest but hands down most efficient way to do it. */
void strprintf(string& str, const char *fmt, ... )
{
	va_list args;
	char *buf = 0;

	va_start(args,fmt);
	if (vasprintf(&buf,fmt,args) >= 0)
		str = buf;
	else
		str = "";
	va_end(args);
	if (buf)
		free(buf);
}