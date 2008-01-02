/* Copyright © 2007, 2008 Jakub Wilk
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991.
 */

#include <string>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <cmath>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocEncoding.h"
#include "splash/SplashBitmap.h"
#include "splash/Splash.h"
#include "splash/SplashFont.h"
#include "splash/SplashGlyphBitmap.h"
#include "splash/SplashPath.h"
#include "GfxState.h"
#include "SplashOutputDev.h"
#include "Link.h"
#include "UTF8.h"

void init_global_params()
{
#if POPPLER_VERSION < 600
  globalParams = new GlobalParams(NULL);
#else
  globalParams = new GlobalParams();
#endif
}

bool set_antialias(bool value)
{
  return globalParams->setAntialias((char*)(value ? "yes" : "no"));
}

PDFDoc *new_document(std::string file_name)
{
  GooString *g_file_name = new GooString(file_name.c_str());
  PDFDoc *doc = new PDFDoc(g_file_name);
  return doc;
}

void set_color(SplashColor &result, uint8_t r, uint8_t g, uint8_t b)
{
#if POPPLER_VERSION < 500
  result.rgb8 = splashMakeRGB8(r, g, b); 
#else
  result[0] = r;
  result[1] = g;
  result[2] = b;
#endif
}

class Renderer : public SplashOutputDev
{
public:
  Renderer(SplashColor &paper_color) :
#if POPPLER_VERSION < 500
    SplashOutputDev(splashModeRGB8Packed, gFalse, paper_color)
#else
    SplashOutputDev(splashModeRGB8, 4, gFalse, paper_color)
#endif
  { }

#if POPPLER_VERSION < 500
  void drawChar(GfxState *state, double x, double y, double dx, double dy, double origin_x, double origin_y, CharCode code, Unicode *unistr, int len)
  {
    this->drawChar(state, x, y, dx, dy, origin_x, origin_y, code, -1, unistr, len);
  }

  virtual void drawChar(GfxState *state, double x, double y, double dx, double dy, double origin_x, double origin_y, CharCode code, int n_bytes, Unicode *unistr, int len)
  {
    this->SplashOutputDev::drawChar(state, x, y, dx, dy, origin_x, origin_y, code, unistr, len);
  }
  
  virtual void drawMaskedImage(GfxState *state, Object *object, Stream *stream, int width, int height, GfxImageColorMap *color_map, Stream *mask_stream, int mask_width, int mask_height, GBool mask_invert) {}
  virtual void drawSoftMaskedImage(GfxState *state, Object *object, Stream *stream, int width, int height, GfxImageColorMap *color_map, Stream *mask_stream, int mask_width, int mask_height,	GfxImageColorMap *mask_color_map) {}

  SplashFont *getCurrentFont()
  {
    return NULL;
  }
#endif  

#if POPPLER_VERSION >= 600
  void processLink(Link *link, Catalog *catalog)
  {
    this->drawLink(link, catalog);
  }

  virtual void drawLink(Link *link, Catalog *catalog) { }
#endif

protected:
  // copied from <PopplerOutputDev.h>
  void convert_path(GfxState *state, SplashPath &splash_path)
  {
    GfxSubpath *subpath;
    GfxPath *path = state->getPath();
    int n_subpaths = path->getNumSubpaths();
    for (int i = 0; i < n_subpaths; i++) 
    {
      subpath = path->getSubpath(i);
      if (subpath->getNumPoints() > 0) 
      {
        double x1, y1, x2, y2, x3, y3;
        state->transform(subpath->getX(0), subpath->getY(0), &x1, &y1);
        splash_path.moveTo((SplashCoord)x1, (SplashCoord)y1);
        int j = 1;
        int n_points = subpath->getNumPoints();
        while (j < n_points)
        {
          if (subpath->getCurve(j)) 
          {
            state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
            state->transform(subpath->getX(j + 1), subpath->getY(j + 1), &x2, &y2);
            state->transform(subpath->getX(j + 2), subpath->getY(j + 2), &x3, &y3);
            splash_path.curveTo((SplashCoord)x1, (SplashCoord)y1, (SplashCoord)x2, (SplashCoord)y2, (SplashCoord)x3, (SplashCoord)y3);
            j += 3;
          } 
          else 
          {
            state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
            splash_path.lineTo((SplashCoord)x1, (SplashCoord)y1);
            j++;
          }
        }
        if (subpath->isClosed())
          splash_path.close();
      }
    }
  }

};

double get_path_area(SplashPath &path)
{
  double area = 0.0;
#if POPPLER_VERSION >= 500
  int path_len = path.getLength();
  double x0, y0;
  Guchar ch;
  path.getPoint(0, &x0, &y0, &ch);
  for (int i = 0; i < path_len - 1; i++)
  {
    double x1, y1, x2, y2;
    path.getPoint(i + 1, &x1, &y1, &ch);
    path.getPoint(i + 2, &x2, &y2, &ch);
    x1 -= x0; y1 -= y0;
    x2 -= x0; y2 -= y0;
    area += x1 * y2 - x2 * y1;
  }
#endif
  return fabs(area);
}

Object *dict_lookup(Object &dict, const char *key, Object *object)
{
  return dict.dictLookup((char*) key, object);
}

Object *dict_lookup(Object *dict, const char *key, Object *object)
{
  return dict->dictLookup((char*) key, object);
}

Object *dict_lookup(Dict *dict, const char *key, Object *object)
{
  return dict->lookup((char*) key, object);
}

double get_page_width(PDFDoc *document, int n)
{
  double width =
#if POPPLER_VERSION < 500
    document->getPageWidth(n);
#else
    document->getPageMediaWidth(n);
#endif
  return width / 72.0;
}

double get_page_height(PDFDoc *document, int n)
{
  double height =
#if POPPLER_VERSION < 500
    document->getPageHeight(n);
#else
    document->getPageMediaHeight(n);
#endif
  return height / 72.0;
}

void display_page(PDFDoc *document, Renderer *renderer, int npage, double hdpi, double vdpi, bool do_links)
{
#if POPPLER_VERSION < 500
  document->displayPage(renderer, npage, hdpi, vdpi, 0, gFalse, do_links);
#elif POPPLER_VERSION < 600 
  document->displayPage(renderer, npage, hdpi, vdpi, 0, gTrue, gFalse, do_links);
#else
  document->displayPage(renderer, npage, hdpi, vdpi, 0, gTrue, gFalse, !do_links);
  document->processLinks(renderer, npage);
#endif
}

class PixmapIterator
{
private:
  const uint8_t *row_ptr;
  const uint8_t *ptr;
  int row_size;
public:  
  PixmapIterator(const uint8_t *raw_data, int row_size)
  {
    this->row_ptr = this->ptr = raw_data;
    this->row_size = row_size;
  }

  PixmapIterator &operator ++(int)
  {
    ptr += 3;
    return *this;
  }

  void next_row()
  {
    ptr = row_ptr = row_ptr + row_size;
  }

  uint8_t operator[](int n)
  {
    return this->ptr[n];
  }
};

class Pixmap
{
private:
  const uint8_t *raw_data;
  SplashBitmap *bmp;
  int row_size;
  int width, height;
public:
  int get_width() { return width; }
  int get_height() { return height; }

  Pixmap(Renderer *renderer)
  {
#if POPPLER_VERSION < 500
    bmp = renderer->getBitmap();
    raw_data = const_cast<const uint8_t*>(bmp->getDataPtr().rgb8p);
#else
    bmp = renderer->takeBitmap();
    raw_data = const_cast<const uint8_t*>(bmp->getDataPtr());
#endif
    width = bmp->getWidth();
    height = bmp->getHeight();
    row_size = bmp->getRowSize();
  }

  ~Pixmap()
  {
#if POPPLER_VERSION >= 500
    delete bmp;
#endif
  }

  PixmapIterator begin()
  {
    return PixmapIterator(raw_data, row_size);
  }

  friend std::ostream &operator<<(std::ostream &, const Pixmap &);
};

std::ostream &operator<<(std::ostream &stream, const Pixmap &pixmap)
{
  int height = pixmap.height;
  int width = pixmap.width;
  int row_size = pixmap.row_size;
  const uint8_t *row_ptr = pixmap.raw_data;
  for (int y = 0; y < height; y++)
  {
    stream.write((const char*) row_ptr, width * 3);
    row_ptr += row_size;
  }
  return stream;
}

std::string get_link_border_color(Link *link)
{
#if POPPLER_VERSION < 600
  double rgb[3];
  LinkBorderStyle *border_style = link->getBorderStyle();
  border_style->getColor(rgb + 0, rgb + 1, rgb + 2);
  std::ostringstream stream;
  stream << "#";
  for (int i = 0; i < 3; i++)
    stream
      << std::setw(2) << std::setfill('0') << std::hex
      << static_cast<int>(rgb[i] * 0xff);
  return stream.str();
#else
  static std::string red("#ff0000");
  // FIXME: find a way to determine link color
  return red;
#endif
}

bool get_glyph(Splash *splash, SplashFont *font, int code, SplashGlyphBitmap *bitmap)
{
  if (font == NULL)
    return false;
#if POPPLER_VERSION >= 602
  SplashClipResult clip_result;
  if (!font->getGlyph(code, 0, 0, bitmap, 0, 0, splash->getClip(), &clip_result))
    return false;
  return (clip_result != splashClipAllOutside);
#else
  return font->getGlyph(code, 0, 0, bitmap); 
#endif
}

class XObject : public Object
{
public:
  ~XObject()
  {
    this->free();
  } 
};

// vim:ts=2 sw=2 et
