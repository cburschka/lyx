// -*- C++ -*-
/**
 * \file GWorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GWORKAREA_H
#define GWORKAREA_H

#include "GPainter.h"
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "frontends/WorkArea.h"

#include <X11/Xft/Xft.h>

class LColor_color;
class LyXView;

namespace lyx {
namespace frontend {

class ColorCache {
public:
	typedef std::map<LColor_color, Gdk::Color *> Map;
	typedef Map::iterator MapIt;
	typedef std::map<LColor_color, XftColor *> Map2;
	typedef Map2::iterator MapIt2;
	~ColorCache();
	Gdk::Color * getColor(LColor_color);
	XftColor * getXftColor(LColor_color);
	void cacheColor(LColor_color, Gdk::Color *);
	void cacheXftColor(LColor_color, XftColor *);
	void clear();
private:
	Map cache_;
	Map2 cache2_;
};

extern ColorCache colorCache;

class ColorHandler {
public:
	ColorHandler(GWorkArea & owner) : owner_(owner) {}
	XftColor * getXftColor(LColor_color clr);
	Gdk::Color * getGdkColor(LColor_color clr);
private:
	GWorkArea & owner_;
};


class GWorkArea : public WorkArea, public SigC::Object {
public:
	GWorkArea(LyXView & owner, int width, int height);
	~GWorkArea();

	virtual Painter & getPainter();
	///
	virtual int workWidth() const;
	///
	virtual int workHeight() const;
	/// return x position of window
	int xpos() const;
	/// return y position of window
	int ypos() const;
	///
	Glib::RefPtr<Gdk::Window> getWindow();
	Display * getDisplay() const;
	Glib::RefPtr<Gdk::Pixmap> getPixmap();
	Glib::RefPtr<Gdk::GC> getGC();
	Glib::RefPtr<Gdk::Colormap> getColormap();
	XftDraw * getXftDraw();
	ColorHandler & getColorHandler();

	virtual void setScrollbarParams(int height, int pos, int line_height);
	/// a selection exists
	virtual void haveSelection(bool) const;
	///
	virtual std::string const getClipboard() const;
	///
	virtual void putClipboard(std::string const &) const;
	void inputCommit(gchar * str);
private:
	bool onExpose(GdkEventExpose * event);
	bool onConfigure(GdkEventConfigure * event);
	void onScroll();
	bool onScrollWheel(GdkEventScroll * event);
	bool onButtonPress(GdkEventButton * event);
	bool onButtonRelease(GdkEventButton * event);
	bool onMotionNotify(GdkEventMotion * event);
	bool onKeyPress(GdkEventKey * event);
	void onClipboardGet(Gtk::SelectionData & selection_data, guint info);
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
	std::string inputCache_;
};

} // namespace frontend
} // namespace lyx

#endif
