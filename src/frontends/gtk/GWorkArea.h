// -*- C++ -*-
/**
 * \file GWorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GWORKAREA_H
#define GWORKAREA_H

#include <gdk/gdkx.h>
#include "frontends/WorkArea.h"
#include "GPainter.h"
#include "LColor.h"
#include <gtk/gtk.h>
#include <X11/Xft/Xft.h>

class ColorCache
{
	typedef std::map<LColor::color, Gdk::Color *> Map;
	typedef Map::iterator MapIt;
	typedef std::map<LColor::color, XftColor *> Map2;
	typedef Map2::iterator MapIt2;
public:
	~ColorCache() { clear(); }
	Gdk::Color * getColor(LColor::color clr)
	{
		MapIt it = cache_.find(clr);
		return it == cache_.end() ? 0 : it->second;
	}
	XftColor * getXftColor(LColor::color clr)
	{
		MapIt2 it = cache2_.find(clr);
		return it == cache2_.end() ? 0 : it->second;
	}
	void cacheColor(LColor::color clr, Gdk::Color * gclr)
	{
		cache_[clr] = gclr;
	}
	void cacheXftColor(LColor::color clr, XftColor * xclr)
	{
		cache2_[clr] = xclr;
	}
	void clear();
private:
	Map cache_;
	Map2 cache2_;
};

extern ColorCache colorCache;

class ColorHandler
{
public:
	ColorHandler(GWorkArea& owner) : owner_(owner) {}
	XftColor * getXftColor(LColor::color clr);
	Gdk::Color * getGdkColor(LColor::color clr);
private:
	GWorkArea & owner_;
};

class GWorkArea : public WorkArea, public SigC::Object
{
public:
	GWorkArea(int width, int height);
	~GWorkArea();

	virtual Painter & getPainter() { return painter_; }
	///
	virtual int workWidth() const { return workArea_.get_width(); }
	///
	virtual int workHeight() const { return workArea_.get_height(); }
	/// return x position of window
	int xpos() const { return 0; }
	/// return y position of window
	int ypos() const { return 0; }
	///
	Glib::RefPtr<Gdk::Window> getWindow() { return workArea_.get_window(); }
	Display * getDisplay() const
	{ return GDK_WINDOW_XDISPLAY(
		const_cast<GdkWindow*>(workArea_.get_window()->gobj())); }
	Glib::RefPtr<Gdk::Pixmap> getPixmap() { return workAreaPixmap_; }
	Glib::RefPtr<Gdk::GC> getGC() { return workAreaGC_; }
	Glib::RefPtr<Gdk::Colormap> getColormap() 
	{ return workArea_.get_colormap(); }
	XftDraw * getXftDraw() { return draw_; }
	ColorHandler & getColorHandler() { return colorHandler_; }

	virtual void setScrollbarParams(int height, int pos, int line_height);
	/// a selection exists
	virtual void haveSelection(bool) const;
	///
	virtual string const getClipboard() const;
	///
	virtual void putClipboard(string const &) const;
	void inputCommit(gchar * str);
private:
	bool onExpose(GdkEventExpose * event);
	bool onConfigure(GdkEventConfigure * event);
	void onScroll();
	bool onButtonPress(GdkEventButton * event);
	bool onButtonRelease(GdkEventButton * event);
	bool onMotionNotify(GdkEventMotion * event);
	bool onKeyPress(GdkEventKey * event);
	void onClipboardGet(Gtk::SelectionData& selection_data, guint info);
	void onClipboardClear();
	Gtk::HBox hbox_;
	Gtk::DrawingArea workArea_;
	Gtk::VScrollbar vscrollbar_;
	/// The pixmap overlay on the workarea
	Glib::RefPtr<Gdk::Pixmap> workAreaPixmap_;
	Glib::RefPtr<Gdk::GC> workAreaGC_;
	/// the xforms-specific painter
	GPainter painter_;
	XftDraw * draw_;
	ColorHandler colorHandler_;
	GtkIMContext * imContext_;
	string inputCache_;
};

#endif
