/**
 * \file GWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GWorkArea.h"
#include "GView.h"
#include "GtkmmX.h"
#include "GLyXKeySym.h"

#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"

using boost::shared_ptr;

using std::string;

namespace lyx {
namespace frontend {

namespace {

mouse_button::state gButtonToLyx(guint gdkbutton)
{
	// GDK uses int 1,2,3 but lyx uses enums (1,2,4)
	switch (gdkbutton) {
	case 1:
		return mouse_button::button1;
	case 2:
		return mouse_button::button2;
	case 3:
		return mouse_button::button3;
	case 4:
		return mouse_button::button4;
	case 5:
		return mouse_button::button5;
	}

	// This shouldn't happen, according to gdk docs
	lyxerr << "gButtonToLyx: unhandled button index\n";
	return mouse_button::button1;
}

} // namespace anon

ColorCache colorCache;

Gdk::Color * ColorCache::getColor(LColor_color clr)
{
	MapIt it = cache_.find(clr);
	return it == cache_.end() ? 0 : it->second.get();
}


XftColor * ColorCache::getXftColor(LColor_color clr)
{
	MapIt2 it = cache2_.find(clr);
	return it == cache2_.end() ? 0 : it->second.get();
}


void ColorCache::cacheColor(LColor_color clr, Gdk::Color * gclr)
{
	cache_[clr] = shared_ptr<Gdk::Color>(gclr);
}


void ColorCache::cacheXftColor(LColor_color clr, XftColor * xclr)
{
	cache2_[clr] = shared_ptr<XftColor>(xclr);
}


void ColorCache::clear()
{
	cache_.clear();
	cache2_.clear();
}


XftColor * ColorHandler::getXftColor(LColor_color clr)
{
	XftColor * xclr = colorCache.getXftColor(clr);
	if (!xclr) {
		xclr = new XftColor;
		Colormap colormap = GDK_COLORMAP_XCOLORMAP(
			owner_.getColormap()->gobj());
		Visual * visual = GDK_VISUAL_XVISUAL(
			owner_.getColormap()->get_visual()->gobj());
		XftColorAllocName(owner_.getDisplay(), visual, colormap,
				  const_cast<char*>(
					  lcolor.getX11Name(clr).c_str())
				  , xclr);
		colorCache.cacheXftColor(clr, xclr);
	}
	return xclr;
}


Gdk::Color * ColorHandler::getGdkColor(LColor_color clr)
{
	Gdk::Color * gclr = colorCache.getColor(clr);
	if (!gclr) {
		gclr = new Gdk::Color;
		gclr->parse(lcolor.getX11Name(clr));
		owner_.getColormap()->alloc_color(*gclr);
		colorCache.cacheColor(clr, gclr);
	}
	return gclr;
}


namespace
{


mouse_button::state gtkButtonState(unsigned int state)
{
	mouse_button::state b = mouse_button::none;
	if (state & GDK_BUTTON1_MASK)
		b = mouse_button::button1;
	else if (state & GDK_BUTTON2_MASK)
		b = mouse_button::button2;
	else if (state & GDK_BUTTON3_MASK)
		b = mouse_button::button3;
	else if (state & GDK_BUTTON3_MASK)
		b = mouse_button::button3;
	else if (state & GDK_BUTTON4_MASK)
		b = mouse_button::button4;
	else if (state & GDK_BUTTON5_MASK)
		b = mouse_button::button5;
	return b;
}


key_modifier::state gtkKeyState(guint state)
{
	key_modifier::state k = key_modifier::none;
	if (state & GDK_CONTROL_MASK)
		k |= key_modifier::ctrl;
	if (state & GDK_SHIFT_MASK)
		k |= key_modifier::shift;
	if (state & GDK_MOD1_MASK)
		k |= key_modifier::alt;
	return k;
}


void inputCommitRelay(GtkIMContext */*imcontext*/, gchar * str, GWorkArea * area)
{
	area->inputCommit(str);
}


}


GWorkArea::GWorkArea(LyXView & owner, int width, int height)
	: workAreaPixmap_(0), painter_(*this), draw_(0), colorHandler_(*this),
	  adjusting_(false)
{
	workArea_.set_size_request(width, height);
	workArea_.set_double_buffered(false);
	workArea_.add_events(Gdk::STRUCTURE_MASK | Gdk::EXPOSURE_MASK |
			     Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::KEY_PRESS_MASK | Gdk::BUTTON1_MOTION_MASK);
	workArea_.signal_expose_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onExpose));
	workArea_.signal_configure_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onConfigure));
	workArea_.signal_button_press_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onButtonPress));
	workArea_.signal_button_release_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onButtonRelease));
	workArea_.signal_key_press_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onKeyPress));
	workArea_.signal_motion_notify_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onMotionNotify));
	workArea_.show();
	vscrollbar_.get_adjustment()->signal_value_changed().connect(
		sigc::mem_fun(*this, &GWorkArea::onScroll));
	workArea_.signal_scroll_event().connect(
		sigc::mem_fun(*this, &GWorkArea::onScrollWheel));
	vscrollbar_.show();
	hbox_.children().push_back(Gtk::Box_Helpers::Element(workArea_));
	hbox_.children().push_back(
		Gtk::Box_Helpers::Element(vscrollbar_,Gtk::PACK_SHRINK));
	hbox_.show();

	GView & gview = static_cast<GView &>(owner);
	gview.getBox(GView::Center).children().push_back(
		Gtk::Box_Helpers::Element(hbox_));

	workArea_.set_flags(workArea_.get_flags() | Gtk::CAN_DEFAULT |
			    Gtk::CAN_FOCUS);
	workArea_.grab_default();
	gview.setGWorkArea(&workArea_);
	imContext_ = GTK_IM_CONTEXT(gtk_im_multicontext_new());
	g_signal_connect(G_OBJECT(imContext_), "commit",
			 G_CALLBACK(&inputCommitRelay),
			 this);
}


GWorkArea::~GWorkArea()
{
	g_object_unref(imContext_);
}


Painter & GWorkArea::getPainter()
{
	return painter_;
}


int GWorkArea::workWidth() const
{
	return workArea_.get_width();
}


int GWorkArea::workHeight() const
{
	return workArea_.get_height();
}


int GWorkArea::xpos() const
{
	return 0;
}


int GWorkArea::ypos() const
{
	return 0;
}


Glib::RefPtr<Gdk::Window> GWorkArea::getWindow()
{
	return workArea_.get_window();
}


Display * GWorkArea::getDisplay() const
{
	return GDK_WINDOW_XDISPLAY(
		const_cast<GdkWindow*>(workArea_.get_window()->gobj()));
}


Glib::RefPtr<Gdk::Pixmap> GWorkArea::getPixmap()
{
	return workAreaPixmap_;
}


Glib::RefPtr<Gdk::GC> GWorkArea::getGC()
{
	return workAreaGC_;
}


Glib::RefPtr<Gdk::Colormap> GWorkArea::getColormap()
{
	return workArea_.get_colormap();
}


XftDraw * GWorkArea::getXftDraw()
{
	return draw_;
}


ColorHandler & GWorkArea::getColorHandler()
{
	return colorHandler_;
}


bool GWorkArea::onExpose(GdkEventExpose * event)
{
	workArea_.get_window()->draw_drawable(
		workArea_.get_style()->get_black_gc(),
		workAreaPixmap_,
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);
	return true;
}


bool GWorkArea::onConfigure(GdkEventConfigure * /*event*/)
{
	int x, y, width, height, depth;
	workArea_.get_window()->get_geometry(x, y, width, height, depth);
	if (draw_)
		XftDrawDestroy(draw_);
	workAreaPixmap_ = Gdk::Pixmap::create(workArea_.get_window(),
					      width, height, depth);
	Pixmap pixmap = GDK_PIXMAP_XID(workAreaPixmap_->gobj());
	Colormap colormap = GDK_COLORMAP_XCOLORMAP(
		workArea_.get_colormap()->gobj());
	Visual * visual = GDK_VISUAL_XVISUAL(
		workArea_.get_colormap()->get_visual()->gobj());
	draw_ = XftDrawCreate(getDisplay(), pixmap,
			      visual, colormap);
	if (!workAreaGC_) {
		workAreaGC_ = Gdk::GC::create(workArea_.get_window());
		Gdk::Cursor cursor(Gdk::XTERM);
		workArea_.get_window()->set_cursor(cursor);
		gtk_im_context_set_client_window(
			imContext_, workArea_.get_window()->gobj());
	}
	workAreaResize();
	return true;
}


void GWorkArea::setScrollbarParams(int height, int pos, int line_height)
{
	if (adjusting_)
		return;

	adjusting_ = true;

	Gtk::Adjustment * adjustment = vscrollbar_.get_adjustment();
	adjustment->set_lower(0);
	int workAreaHeight = workHeight();
	if (!height || height < workAreaHeight) {
		adjustment->set_upper(workAreaHeight);
		adjustment->set_page_size(workAreaHeight);
		adjustment->set_value(0);
		adjustment->changed();
		adjusting_ = false;
		return;
	}
	adjustment->set_step_increment(line_height * 3);
	adjustment->set_page_increment(workAreaHeight - line_height);
	// Allow the user half a screen of blank at the end
	// to make scrollbar consistant with centering the cursor
	adjustment->set_upper(height + workAreaHeight / 4);
	adjustment->set_page_size(workAreaHeight);
	adjustment->set_value(pos);
	adjustment->changed();
	adjusting_ = false;
}


void GWorkArea::onScroll()
{
	if (adjusting_)
		return;

	adjusting_ = true;

	double val = vscrollbar_.get_adjustment()->get_value();
	scrollDocView(static_cast<int>(val));
	adjusting_ = false;
}


bool GWorkArea::onScrollWheel(GdkEventScroll * event)
{
	Gtk::Adjustment * adjustment = vscrollbar_.get_adjustment();

	double step;
	if (event->state & GDK_CONTROL_MASK)
		step = adjustment->get_page_increment();
	else
		step = adjustment->get_step_increment();

	if (event->direction == GDK_SCROLL_UP)
		step *= -1.0f;

	double target = adjustment->get_value() + step;
	// Prevent the user getting a whole screen of blank when they
	// try to scroll past the end of the doc
	double max = adjustment->get_upper() - workHeight();
	if (target > max)
		target = max;

	adjustment->set_value(target);
	return true;
}


bool GWorkArea::onButtonPress(GdkEventButton * event)
{
	kb_action ka = LFUN_MOUSE_PRESS;
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		ka = LFUN_MOUSE_PRESS;
		break;
	case GDK_2BUTTON_PRESS:
		ka = LFUN_MOUSE_DOUBLE;
		break;
	case GDK_3BUTTON_PRESS:
		ka = LFUN_MOUSE_TRIPLE;
		break;
	default:
		break;
	}
	dispatch(FuncRequest(ka,
			     static_cast<int>(event->x),
			     static_cast<int>(event->y),
			     gButtonToLyx(event->button)));
	workArea_.grab_focus();
	return true;
}


bool GWorkArea::onButtonRelease(GdkEventButton * event)
{
	dispatch(FuncRequest(LFUN_MOUSE_RELEASE,
			     static_cast<int>(event->x),
			     static_cast<int>(event->y),
			     gButtonToLyx(event->button)));
	return true;
}


bool GWorkArea::onMotionNotify(GdkEventMotion * event)
{
	static guint32 timeBefore;
	Gtk::Adjustment * adjustment = vscrollbar_.get_adjustment();
	double step = adjustment->get_step_increment();
	double value = adjustment->get_value();
	if (event->x < 0)
		value -= step;
	else if (event->x > workArea_.get_height())
		value += step;
	if (value != adjustment->get_value()) {
		if (event->time - timeBefore > 200) {
			adjustment->set_value(value);
			adjustment->value_changed();
		}
		timeBefore = event->time;
	}
	dispatch(FuncRequest(LFUN_MOUSE_MOTION,
			     static_cast<int>(event->x),
			     static_cast<int>(event->y),
			     gtkButtonState(event->state)));
	return true;
}


void GWorkArea::inputCommit(gchar * str)
{
	inputCache_ = Glib::locale_from_utf8(str);
}


bool GWorkArea::onKeyPress(GdkEventKey * event)
{
#ifdef I18N
	inputCache_ = "";
	bool inputGet = gtk_im_context_filter_keypress(imContext_, event);
	// cope with ascii
	if ((inputGet && inputCache_.size() == 1 && inputCache_[0] < 128) ||
	    !inputGet) {
#endif
		GLyXKeySym *glk = new GLyXKeySym(event->keyval);
		workAreaKeyPress(LyXKeySymPtr(glk),
				 gtkKeyState(event->state));
#ifdef I18N
	} else if (!inputCache_.empty())
		workAreaCJK_IMprocess(inputCache_.size(), inputCache_.data());
#endif
	return true;
}


void GWorkArea::onClipboardGet(Gtk::SelectionData & /*selection_data*/,
			       guint /*info*/)
{
	selectionRequested();
}


void GWorkArea::onClipboardClear()
{
//	selectionLost();
}


void GWorkArea::haveSelection(bool toHave) const
{
	if (toHave) {
		Glib::RefPtr<Gtk::Clipboard> clipboard =
			Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
		std::vector<Gtk::TargetEntry> listTargets;
		listTargets.push_back(Gtk::TargetEntry("UTF8_STRING"));
		clipboard->set(listTargets,
			       sigc::mem_fun(const_cast<GWorkArea&>(*this),
					  &GWorkArea::onClipboardGet),
			       sigc::mem_fun(const_cast<GWorkArea&>(*this),
					  &GWorkArea::onClipboardClear));
	}
}


string const GWorkArea::getClipboard() const
{
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	return Glib::locale_from_utf8(clipboard->wait_for_text());
}


void GWorkArea::putClipboard(string const & str) const
{
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	clipboard->set_text(Glib::locale_to_utf8(str));
}

} // namespace frontend
} // namespace lyx
