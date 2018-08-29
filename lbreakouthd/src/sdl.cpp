/*
 * sdl.cpp
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
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "tools.h"
#include "sdl.h"

int Geom::sw = 640; /* safe values before MainWindow is called */
int Geom::sh = 480;
bool Image::useColorKeyBlack = false; /* workaround for old color key in lbr2 themes */

SDL_Renderer *mrc = NULL; /* main window render context, got only one */

/** Main application window */

MainWindow::MainWindow(const char *title, int _w, int _h, int _full)
{
	int flags = 0;
	if (_w <= 0 || _h <= 0) { /* no width or height, use desktop setting */
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(0,&mode);
		_w = mode.w;
		_h = mode.h;
		_full = 1;
	}
	if (_full)
		flags = SDL_WINDOW_FULLSCREEN;
	w = _w;
	h = _h;
	Geom::sw = w;
	Geom::sh = h;
	if( (mw = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED, w, h, flags)) == NULL)
		_logsdlerr();
	if ((mr = SDL_CreateRenderer(mw, -1, SDL_RENDERER_ACCELERATED)) == NULL)
		_logsdlerr();
	mrc = mr;
}
MainWindow::~MainWindow()
{
	if (mr)
		SDL_DestroyRenderer(mr);
	if (mw)
		SDL_DestroyWindow(mw);
}

void MainWindow::refresh()
{
	SDL_RenderPresent(mr);
}

/** Image */

int Image::create(int w, int h)
{
	if (tex) {
		SDL_DestroyTexture(tex);
		tex = NULL;
	}

	this->w = w;
	this->h = h;
	if ((tex = SDL_CreateTexture(mrc,SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET,w, h)) == NULL) {
		_logsdlerr();
		return 0;
	}
	fill(0,0,0,0);
	setBlendMode(1);
	return 1;
}

int Image::createFromScreen()
{
	int w, h;

	SDL_GetRendererOutputSize(mrc,&w,&h);
	SDL_Surface *sshot = SDL_CreateRGBSurface(0, w, h, 32,
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(mrc, NULL, SDL_PIXELFORMAT_ARGB8888,
					sshot->pixels, sshot->pitch);
	return load(sshot);
}

int Image::load(const string& fname)
{
	if (tex) {
		SDL_DestroyTexture(tex);
		tex = NULL;
	}

	SDL_Surface *surf = IMG_Load(fname.c_str());
	if (surf == NULL) {
		_logsdlerr();
		return 0;
	}
	if (Image::useColorKeyBlack)
		SDL_SetColorKey(surf, SDL_TRUE, 0x0);
	if ((tex = SDL_CreateTextureFromSurface(mrc, surf)) == NULL) {
		_logsdlerr();
		return 0;
	}
	w = surf->w;
	h = surf->h;
	SDL_FreeSurface(surf);
	return 1;
}
int Image::load(SDL_Surface *s)
{
	if (tex) {
		SDL_DestroyTexture(tex);
		tex = NULL;
	}

	if ((tex = SDL_CreateTextureFromSurface(mrc,s)) == NULL) {
		_logsdlerr();
		return 0;
	}
	w = s->w;
	h = s->h;
	return 1;
}
int Image::load(Image *s, int x, int y, int w, int h)
{
	if (tex) {
		SDL_DestroyTexture(tex);
		tex = NULL;
	}

	SDL_Rect srect = {x, y, w, h};
	SDL_Rect drect = {0, 0, w, h};

	this->w = w;
	this->h = h;
	if ((tex = SDL_CreateTexture(mrc,SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET,w, h)) == NULL) {
		_logsdlerr();
		return 0;
	}
	SDL_SetRenderTarget(mrc, tex);
	SDL_RenderCopy(mrc, s->getTex(), &srect, &drect);
	SDL_SetRenderTarget(mrc, NULL);
	return 1;
}

SDL_Texture *Image::getTex()
{
	return tex;
}

void Image::copy() /* full scale */
{
	if (tex == NULL)
		return;

	SDL_RenderCopy(mrc, tex, NULL, NULL);
}
void Image::copy(int dx, int dy)
{
	if (tex == NULL)
		return;

	SDL_Rect drect = {dx, dy, w, h};
	SDL_RenderCopy(mrc, tex, NULL, &drect);
}
void Image::copy(int dx, int dy, int dw, int dh)
{
	if (tex == NULL)
		return;

	SDL_Rect drect = {dx, dy, dw, dh};
	SDL_RenderCopy(mrc, tex, NULL, &drect);
}
void Image::copy(int sx, int sy, int sw, int sh, int dx, int dy) {
	if (tex == NULL)
		return;

	SDL_Rect srect = {sx, sy, sw, sh};
	SDL_Rect drect = {dx, dy, sw, sh};
	SDL_RenderCopy(mrc, tex, &srect, &drect);
}

void Image::fill(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_Texture *old = SDL_GetRenderTarget(mrc);
	SDL_SetRenderTarget(mrc,tex);
	SDL_SetRenderDrawColor(mrc,r,g,b,a);
	SDL_RenderClear(mrc);
	SDL_SetRenderTarget(mrc,old);
}

void Image::scale(int nw, int nh)
{
	if (tex == NULL)
		return;
	if (nw == w && nh == h)
		return; /* already ok */

	SDL_Texture *newtex = 0;
	if ((newtex = SDL_CreateTexture(mrc,SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_TARGET,nw,nh)) == NULL) {
		_logsdlerr();
		return;
	}
	SDL_Texture *oldTarget = SDL_GetRenderTarget(mrc);
	SDL_SetRenderTarget(mrc, newtex);
	SDL_SetTextureBlendMode(newtex, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(mrc,0,0,0,0);
	SDL_RenderClear(mrc);
	SDL_RenderCopy(mrc, tex, NULL, NULL);
	SDL_SetRenderTarget(mrc, oldTarget);

	SDL_DestroyTexture(tex);
	tex = newtex;
	w = nw;
	h = nh;
}

/** Create shadow image by clearing all r,g,b to 0 and cutting alpha in half. */
int Image::createShadow(Image &img)
{
	/* duplicate first */
	create(img.getWidth(),img.getHeight());
	SDL_Texture *oldTarget = SDL_GetRenderTarget(mrc);
	SDL_SetRenderTarget(mrc, tex);
	img.copy(0,0);

	/* change and apply */
	Uint32 pixels[w*h];
	int pitch = w*sizeof(Uint32);
	Uint8 r,g,b,a;
	Uint32 pft = 0;
	SDL_QueryTexture(tex, &pft, 0, 0, 0);
	SDL_PixelFormat* pf = SDL_AllocFormat(pft);
	if (SDL_RenderReadPixels(mrc, NULL, pft, pixels, pitch) < 0) {
		_logsdlerr();
		return 0;
	}
	for (int i = 0; i < w * h; i++) {
		SDL_GetRGBA(pixels[i],pf,&r,&g,&b,&a);
		pixels[i] = SDL_MapRGBA(pf,0,0,0,a/2);
	}
	SDL_UpdateTexture(tex, NULL, pixels, pitch);
	SDL_FreeFormat(pf);
	SDL_SetRenderTarget(mrc, oldTarget);

	return 1;
}

/** Grid image: large bitmap with same sized icons */

int GridImage::load(const string& fname, int _gw, int _gh)
{
	gw = _gw;
	gh = _gh;
	return Image::load(fname);
}

int GridImage::load(SDL_Surface *s, int _gw, int _gh)
{
	gw = _gw;
	gh = _gh;
	return Image::load(s);
}

void GridImage::copy(int gx, int gy, int dx, int dy)
{
	if (tex == NULL )
		return;

	SDL_Rect srect = {gx * gw, gy * gh, gw, gh};
	SDL_Rect drect = {dx, dy, gw, gh};
	SDL_RenderCopy(mrc, tex, &srect, &drect);
}
void GridImage::copy(int gx, int gy, int dx, int dy, int dw, int dh)
{
	if (tex == NULL )
		return;

	SDL_Rect srect = {gx * gw, gy * gh, gw, gh};
	SDL_Rect drect = {dx, dy, dw, dh};
	SDL_RenderCopy(mrc, tex, &srect, &drect);
}
void GridImage::copy(int gx, int gy, int sx, int sy, int sw, int sh, int dx, int dy)
{
	if (tex == NULL )
		return;

	SDL_Rect srect = {gx * gw + sx, gy * gh + sy, sw, sh};
	SDL_Rect drect = {dx, dy, sw, sh};
	SDL_RenderCopy(mrc, tex, &srect, &drect);

}


/** Scale cell by cell to prevent artifacts. */
void GridImage::scale(int ncw, int nch)
{
	if (tex == NULL)
		return;

	int nw = ncw * getGridSizeX();
	int nh = nch * getGridSizeY();
	SDL_Texture *newtex = 0;
	SDL_Rect srect = {0, 0, gw, gh};
	SDL_Rect drect = {0, 0, ncw, nch};

	if (nw == w && nh == h)
		return; /* already ok */

	if ((newtex = SDL_CreateTexture(mrc,SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_TARGET,nw,nh)) == NULL) {
		_logsdlerr();
		return;
	}
	SDL_SetRenderTarget(mrc, newtex);
	SDL_SetTextureBlendMode(newtex, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(mrc,0,0,0,0);
	SDL_RenderClear(mrc);
	for (int j = 0; j < getGridSizeY(); j++)
		for (int i = 0; i < getGridSizeX(); i++) {
			srect.x = i * gw;
			srect.y = j * gh;
			drect.x = i * ncw;
			drect.y = j * nch;
			SDL_RenderCopy(mrc, tex, &srect, &drect);
		}
	SDL_SetRenderTarget(mrc, NULL);

	SDL_DestroyTexture(tex);
	tex = newtex;
	w = nw;
	h = nh;
	gw = ncw;
	gh = nch;
}

int GridImage::createShadow(GridImage &img)
{
	gw = img.getGridWidth();
	gh = img.getGridHeight();
	return Image::createShadow(img);
}


/** Font */

Font::Font() : font(0), size(0)
{
}
Font::~Font() {
	if (font)
		TTF_CloseFont(font);
}

void Font::load(const string& fname, int sz) {
	if (font) {
		TTF_CloseFont(font);
		font = 0;
		size = 0;
	}

	if ((font = TTF_OpenFont(fname.c_str(), sz)) == NULL) {
		_logsdlerr();
		return;
	}
	size = sz;
	setColor(255,255,255,255);
	setAlign(ALIGN_X_LEFT | ALIGN_Y_TOP);
}

void Font::setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_Color c = {r, g, b, a};
	clr = c;
}
void Font::setColor(SDL_Color c) {
	clr = c;
}
void Font::setAlign(int a) {
	align = a;
}
void Font::write(int x, int y, const string& _str) {
	if (font == 0)
		return;

	/* XXX doesn't look good, why no rendering
	 * into texture directly available? */
	SDL_Surface *surf;
	SDL_Texture *tex;
	SDL_Rect drect;
	const char *str = _str.c_str();

	if (strlen(str) == 0)
		return;

	if ((surf = TTF_RenderUTF8_Blended(font, str, clr)) == NULL)
		_logsdlerr();
	if ((tex = SDL_CreateTextureFromSurface(mrc, surf)) == NULL)
		_logsdlerr();
	if (align & ALIGN_X_LEFT)
		drect.x = x;
	else if (align & ALIGN_X_RIGHT)
		drect.x = x - surf->w;
	else
		drect.x = x - surf->w/2; /* center */
	if (align & ALIGN_Y_TOP)
		drect.y = y;
	else if (align & ALIGN_Y_BOTTOM)
		drect.y = y - surf->h;
	else
		drect.y = y - surf->h/2;
	drect.w = surf->w;
	drect.h = surf->h;
	SDL_RenderCopy(mrc, tex, NULL, &drect);
}
void Font::writeText(int x, int y, const string& _text, int wrapwidth)
{
	if (font == 0)
		return;

	/* XXX doesn't look good, why no rendering
	 * into texture directly available? */
	SDL_Surface *surf;
	SDL_Texture *tex;
	SDL_Rect drect;
	const char *text = _text.c_str();

	if (strlen(text) == 0)
		return;

	if ((surf = TTF_RenderUTF8_Blended_Wrapped(font, text, clr, wrapwidth)) == NULL)
		_logsdlerr();
	if ((tex = SDL_CreateTextureFromSurface(mrc, surf)) == NULL)
		_logsdlerr();
	drect.x = x;
	drect.y = y;
	drect.w = surf->w;
	drect.h = surf->h;
	SDL_RenderCopy(mrc, tex, NULL, &drect);
}

