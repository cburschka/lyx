/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlGraphics.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 * \author Herbert Voss <voss@perce.de>
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlGraphics.h"
#include "ControlInset.tmpl"

#include "helper_funcs.h"

#include "converter.h"
#include "buffer.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "gettext.h"
#include "lyxrc.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsConverter.h"

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"

#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/FileInfo.h"


using std::pair;
using std::make_pair;
using std::ifstream;
using std::vector;

namespace {

// FIXME: currently we need the second '|' to prevent mis-interpretation!
// All supported graphic formats with their file-extension and the
// gzip-ext for zipped (e)ps-files.
// string const grfx_pattern = 
// 	"*.(agr|bmp|eps|epsi|fits|gif|jpg|obj|pdf|pbm|pgm|png|"
// 	"ppm|ps|tif|tiff|xbm|xpm|xwd|gz)|";
vector<string> const grfx_formats()
{
	vector<string> native_formats = grfx::GCache::get().loadableFormats();
	// We can load any format that can be loaded natively together with
	// those that can be converted to one of these native formats.
	vector<string> browsable_formats = native_formats;
	
	grfx::GConverter const & gconverter = grfx::GConverter::get();
	
	vector<string>::const_iterator to_end = native_formats.end();

	Formats::const_iterator from_it = formats.begin();
	Formats::const_iterator from_end = formats.end();
	for (; from_it != from_end; ++from_it) {
		string const from = from_it->name();
		
		vector<string>::const_iterator to_it = native_formats.begin();
		for (; to_it != to_end; ++to_it) {
			if (gconverter.isReachable(from, *to_it)) {
				browsable_formats.push_back(from);
				break;
			}
		}
	}

	browsable_formats.push_back("gz");

	return browsable_formats;
}


string const xforms_pattern()
{
	vector<string> const browsable_formats = grfx_formats();
	string const answer = 
		"*.(" + getStringFromVector(browsable_formats, "|") +")|";
	return answer;
}

}

 
ControlGraphics::ControlGraphics(LyXView & lv, Dialogs & d)
	: ControlInset<InsetGraphics, InsetGraphicsParams>(lv, d)
{
	d_.showGraphics.connect(SigC::slot(this, &ControlGraphics::showInset));
}


InsetGraphicsParams const ControlGraphics::getParams(string const &)
{
	return InsetGraphicsParams();
}


InsetGraphicsParams const
ControlGraphics::getParams(InsetGraphics const & inset)
{
	return inset.params();
}


void ControlGraphics::applyParamsToInset()
{
	// Set the parameters in the inset, it also returns true if the new
	// parameters are different from what was in the inset already.
	bool changed = inset()->setParams(params(), lv_.buffer()->filePath());
	// Tell LyX we've got a change, and mark the document dirty,
	// if it changed.
	lv_.view()->updateInset(inset(), changed);
}


void ControlGraphics::applyParamsNoInset()
{}


// We need these in the file browser.
extern string system_lyxdir;
extern string user_lyxdir;


string const ControlGraphics::Browse(string const & in_name)
{
	string const title = _("Select graphics file");

	// Does user clipart directory exist?
	string clipdir = AddName (user_lyxdir, "clipart");
	FileInfo fileInfo(clipdir);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		clipdir = AddName (system_lyxdir, "clipart");
	pair<string, string> dir1(_("Clipart|#C#c"), clipdir);
	pair<string, string> dir2(_("Documents|#o#O"), string(lyxrc.document_path));
	// Show the file browser dialog
	return browseRelFile(&lv_, in_name, lv_.buffer()->filePath(),
			     title, ::xforms_pattern(), dir1, dir2);
}


string const ControlGraphics::readBB(string const & file)
{
	string const abs_file = MakeAbsPath(file, lv_.buffer()->filePath());

	string const from = getExtFromContents(abs_file);	
	// Check if we have a Postscript file, then it's easy
	if (contains(from, "ps"))
		return readBB_from_PSFile(abs_file);

	// we don't, so ask the Graphics Cache if it has loaded the file
	grfx::GCache & gc = grfx::GCache::get();
	return ("0 0 " + 
		tostr(gc.raw_width(abs_file)) + ' ' + 
		tostr(gc.raw_height(abs_file)));
}


bool ControlGraphics::isFilenameValid(string const & fname) const
{
	// It may be that the filename is relative.
	string const name = MakeAbsPath(fname, lv_.buffer()->filePath());
	return IsFileReadable(name);
}


namespace frnt {

namespace {

// These are the strings that are stored in the LyX file and which
// correspond to the LaTeX identifiers shown in the comments at the
// end of each line.
char const * const rorigin_lyx_strs[] = {
	"center",                                        // c
	"leftTop",   "leftBottom",   "leftBaseline",     // lt lb lB
	"centerTop", "centerBottom", "centerBaseline",   // ct cb cB
	"rightTop",  "rightBottom",  "rightBaseline" };  // rt rb rB

// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const rorigin_gui_strs[] = {
	N_("center"),
	N_("left top"),   N_("left bottom"),   N_("left baseline"),
	N_("center top"), N_("center bottom"), N_("center baseline"),
	N_("right top"),  N_("right bottom"),  N_("right baseline") };

size_t rorigin_size = sizeof(rorigin_lyx_strs) / sizeof(char *);

} // namespace anon

vector<RotationOriginPair> getRotationOriginData()
{
	static vector<RotationOriginPair> data;
	if (!data.empty())
		return data;

	data.resize(rorigin_size);
	for (lyx::size_type i = 0; i < rorigin_size; ++i) {
		data[i] = std::make_pair(_(rorigin_gui_strs[i]),
					 rorigin_lyx_strs[i]);
	}

	return data;
}

} // namespace frnt

