/**
 * \file lyx_gui.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "lyx_gui.h"

#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"

#include "debug.h"
#include "gettext.h"

#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"

// FIXME: move this stuff out again
#include "bufferlist.h"
#include "lyxfunc.h"
#include "lyxserver.h"
#include "BufferView.h"
#include "XFormsView.h"

#include FORMS_H_LOCATION
#include "ColorHandler.h"
#include "xforms_helpers.h"
#include "xfont_loader.h"
#ifdef USE_XFORMS_IMAGE_LOADER
#include "xformsImage.h"
#else
#include "graphics/GraphicsImageXPM.h"
#endif

#include "Lsstream.h"
#include <iomanip>
#include <fcntl.h>
#include <boost/bind.hpp>

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::vector;
using std::hex;
using std::endl;
using std::setbase;
using std::setfill;
using std::setw;

extern BufferList bufferlist;

// FIXME: wrong place !
LyXServer * lyxserver;

namespace {

/// quit lyx
bool finished = false;

/// set default GUI configuration
void setDefaults()
{
	FL_IOPT cntl;
	cntl.buttonFontSize = FL_NORMAL_SIZE;
	cntl.browserFontSize = FL_NORMAL_SIZE;
	cntl.labelFontSize = FL_NORMAL_SIZE;
	cntl.choiceFontSize = FL_NORMAL_SIZE;
	cntl.inputFontSize = FL_NORMAL_SIZE;
	cntl.menuFontSize  = FL_NORMAL_SIZE;
	cntl.borderWidth = -1;
	cntl.vclass = FL_DefaultVisual;
	fl_set_defaults(FL_PDVisual
			| FL_PDButtonFontSize
			| FL_PDBrowserFontSize
			| FL_PDLabelFontSize
			| FL_PDChoiceFontSize
			| FL_PDInputFontSize
			| FL_PDMenuFontSize
			| FL_PDBorderWidth, &cntl);
}


extern "C" {

int LyX_XErrHandler(Display * display, XErrorEvent * xeev) {
	// We don't abort on BadWindow
	if (xeev->error_code == BadWindow) {
		lyxerr << "BadWindow received !" << endl;
		lyxerr << "If you're using xforms 1.0 or greater, "
			<< " please report this to lyx-devel@lists.lyx.org" << endl;
		return 0;
	}

	// emergency cleanup
	LyX::emergencyCleanup();

	// Get the reason for the crash.
	char etxt[513];
	XGetErrorText(display, xeev->error_code, etxt, 512);
	lyxerr << etxt << " id: " << xeev->resourceid << endl;
	// By doing an abort we get a nice backtrace. (hopefully)
	lyx::abort();
	return 0;
}

}

/// read in geometry specification
char geometry[40];

} // namespace anon


void lyx_gui::parse_init(int & argc, char * argv[])
{
	setDefaults();

	FL_CMD_OPT cmdopt[] = {
		{"-geometry", "*.geometry", XrmoptionSepArg, "690x510"}
	};

	FL_resource res[] = {
		{"geometry", "geometryClass", FL_STRING, geometry, "", 40}
	};

	const int num_res = sizeof(res)/sizeof(FL_resource);

	fl_initialize(&argc, argv, "LyX", cmdopt, num_res);

	// It appears that, in xforms >=0.89.5, fl_initialize()
	// calls setlocale() and ruins our LC_NUMERIC setting.
	locale_init();

	fl_get_app_resources(res, num_res);

	Display * display = fl_get_display();

	if (!display) {
		lyxerr << "LyX: unable to access X display, exiting" << endl;
		os::warn("Unable to access X display, exiting");
		::exit(1);
	}

	fcntl(ConnectionNumber(display), F_SETFD, FD_CLOEXEC);

	XSetErrorHandler(LyX_XErrHandler);

	lyxColorHandler.reset(new LyXColorHandler());
 
	using namespace grfx;

#ifdef USE_XFORMS_IMAGE_LOADER
	// connect the image loader based on the xforms library
	Image::newImage = boost::bind(&xformsImage::newImage);
	Image::loadableFormats = boost::bind(&xformsImage::loadableFormats);
#else
	// connect the image loader based on the XPM library
	Image::newImage = boost::bind(&ImageXPM::newImage);
	Image::loadableFormats = boost::bind(&ImageXPM::loadableFormats);
#endif
}


void lyx_gui::parse_lyxrc()
{
	// FIXME !!!!
	lyxrc.dpi = 95;

	XformsColor::read(AddName(user_lyxdir, "preferences.xform"));

	if (lyxrc.popup_font_encoding.empty())
		lyxrc.popup_font_encoding = lyxrc.font_norm;
	// Set the font name for popups and menus
	string boldfontname = lyxrc.popup_bold_font
			       + "-*-*-*-?-*-*-*-*-"
			       + lyxrc.popup_font_encoding;
		// "?" means "scale that font"
	string fontname = lyxrc.popup_normal_font
			       + "-*-*-*-?-*-*-*-*-"
			       + lyxrc.popup_font_encoding;

	int bold = fl_set_font_name(FL_BOLD_STYLE, boldfontname.c_str());
	int normal = fl_set_font_name(FL_NORMAL_STYLE, fontname.c_str());
	if (bold < 0)
		lyxerr << "Could not set menu font to "
		       << boldfontname << endl;

	if (normal < 0)
		lyxerr << "Could not set popup font to "
		       << fontname << endl;

	if (bold < 0 && normal < 0) {
		lyxerr << "Using 'helvetica' font for menus" << endl;
		boldfontname = "-*-helvetica-bold-r-*-*-*-?-*-*-*-*-iso8859-1";
		fontname = "-*-helvetica-medium-r-*-*-*-?-*-*-*-*-iso8859-1";
		bold = fl_set_font_name(FL_BOLD_STYLE, boldfontname.c_str());
		normal = fl_set_font_name(FL_NORMAL_STYLE, fontname.c_str());

		if (bold < 0 && normal < 0) {
			lyxerr << "Could not find helvetica font. Using 'fixed'." << endl;
			fl_set_font_name(FL_NORMAL_STYLE, "fixed");
			normal = bold = 0;
		}
	}
	if (bold < 0)
		fl_set_font_name(FL_BOLD_STYLE, fontname.c_str());
	else if (normal < 0)
		fl_set_font_name(FL_NORMAL_STYLE, boldfontname.c_str());

	fl_setpup_fontstyle(FL_NORMAL_STYLE);
	fl_setpup_fontsize(FL_NORMAL_SIZE);
	fl_setpup_color(FL_MCOL, FL_BLACK);
	fl_set_goodies_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);
#if FL_REVISION < 89
	fl_set_oneliner_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);
#else
	fl_set_tooltip_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);
#endif
}


void lyx_gui::start(string const & batch, vector<string> files)
{
	// initial geometry
	int xpos = -1;
	int ypos = -1;
	unsigned int width = 690;
	unsigned int height = 510;

	static const int geometryBitmask =
		XParseGeometry(geometry,
				&xpos, &ypos, &width, &height);

	// if width is not set by geometry, check it against monitor width
	if (!(geometryBitmask & 4)) {
		Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen);
		if (WidthOfScreen(scr) - 8 < int(width))
			width = WidthOfScreen(scr) - 8;
	}

	// if height is not set by geometry, check it against monitor height
	if (!(geometryBitmask & 8)) {
		Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen);
		if (HeightOfScreen(scr) - 24 < int(height))
			height = HeightOfScreen(scr) - 24;
	}

	Screen * s = ScreenOfDisplay(fl_get_display(), fl_screen);

	// recalculate xpos if it's not set
	if (xpos == -1)
		xpos = (WidthOfScreen(s) - width) / 2;

	// recalculate ypos if it's not set
	if (ypos == -1)
		ypos = (HeightOfScreen(s) - height) / 2;

	lyxerr[Debug::GUI] << "Creating view: " << width << "x" << height
		<< "+" << xpos << "+" << ypos << endl;

	XFormsView view(width, height);
	view.show(xpos, ypos, "LyX");
	view.init();

	Buffer * last = 0;

	// FIXME: some code below needs moving

	lyxserver = new LyXServer(view.getLyXFunc(), lyxrc.lyxpipes);

	vector<string>::const_iterator cit = files.begin();
	vector<string>::const_iterator end = files.end();
	for (; cit != end; ++cit) {
		Buffer * b = bufferlist.loadLyXFile(*cit);
		if (b) {
			last = b;
		}
	}

	// switch to the last buffer successfully loaded
	if (last) {
		view.view()->buffer(last);
	}

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view.getLyXFunc()->dispatch(batch);
	}

	// enter the event loop
	while (!finished) {
		if (fl_check_forms() == FL_EVENT) {
			XEvent ev;
			fl_XNextEvent(&ev);
			lyxerr << "Received unhandled X11 event" << endl;
			lyxerr << "Type: 0x" << hex << ev.xany.type <<
				" Target: 0x" << hex << ev.xany.window << endl;
		}
	}

	// FIXME: breaks emergencyCleanup
	delete lyxserver;
}


void lyx_gui::exit()
{
	finished = true;
}

 
string const lyx_gui::hexname(LColor::color col)
{
	string const name = lcolor.getX11Name(col);
	Display * const display = fl_get_display();
	Colormap const cmap = fl_state[fl_get_vclass()].colormap;
	XColor xcol, ccol;

	if (XLookupColor(display, cmap, name.c_str(), &xcol, &ccol) == 0) {
			lyxerr << "X can't find color \""
			       << lcolor.getLyXName(col)
			       << "\"" << endl;
			return string();
	}

	ostringstream os;

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	os << setbase(16) << setfill('0')
	   << setw(2) << (xcol.red   / 256)
	   << setw(2) << (xcol.green / 256)
	   << setw(2) << (xcol.blue  / 256);

	return os.str().c_str();
}


void lyx_gui::update_color(LColor::color col)
{
	lyxColorHandler->updateColor(col);
}


void lyx_gui::update_fonts()
{
	fontloader.update();
}


bool lyx_gui::font_available(LyXFont const & font)
{
	return fontloader.available(font);
}

namespace {

extern "C"
void C_read_callback(int, void * data)
{
	LyXComm * comm = static_cast<LyXComm *>(data); 
	comm->read_ready();
}

}

void lyx_gui::set_read_callback(int fd, LyXComm * comm)
{
	fl_add_io_callback(fd, FL_READ, C_read_callback, comm);
}
