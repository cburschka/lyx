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
 * \author Angus Leeming <leeming@lyx.org>
 * \author Herbert Voss <voss@perce.de>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlGraphics.h"

#include "helper_funcs.h"

#include "buffer.h"
#include "BufferView.h"
#include "converter.h"
#include "gettext.h"
#include "lyxrc.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"

#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/FileInfo.h"

using std::pair;
using std::make_pair;
using std::vector;

ControlGraphics::ControlGraphics(LyXView & lv, Dialogs & d)
	: ControlInset<InsetGraphics, InsetGraphicsParams>(lv, d)
{}


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
	bool changed = inset()->setParams(params(), buffer()->filePath());

	// Tell LyX we've got a change, and mark the document dirty,
	// if it changed.
	bufferview()->updateInset(inset(), changed);
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
	return browseRelFile(&lv_, in_name, buffer()->filePath(),
			     title, "*.*", dir1, dir2);
}


string const ControlGraphics::readBB(string const & file)
{
	string const abs_file = MakeAbsPath(file, buffer()->filePath());

	// try to get it from the file, if possible. Zipped files are
	// unzipped in the readBB_from_PSFile-Function
	string const bb = readBB_from_PSFile(abs_file);
	if (!bb.empty())
		return bb;

	// we don't, so ask the Graphics Cache if it has loaded the file
	int width = 0;
	int height = 0;

	grfx::Cache & gc = grfx::Cache::get();
	if (gc.inCache(abs_file)) {
		grfx::Image const * image = gc.item(abs_file)->image();

		if (image) {
			width  = image->getWidth();
			height = image->getHeight();
		}
	}

	return ("0 0 " + tostr(width) + ' ' + tostr(height));
}


bool ControlGraphics::isFilenameValid(string const & fname) const
{
	// It may be that the filename is relative.
	string const name = MakeAbsPath(fname, buffer()->filePath());
	return IsFileReadable(name);
}


namespace frnt {

namespace {

// These are the strings that are stored in the LyX file and which
// correspond to the LaTeX identifiers shown in the comments at the
// end of each line.
char const * const rorigin_lyx_strs[] = {
	// the LaTeX default is leftBaseline
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

size_t const rorigin_size = sizeof(rorigin_lyx_strs) / sizeof(char *);

} // namespace anon


vector<RotationOriginPair> getRotationOriginData()
{
	static vector<RotationOriginPair> data;
	if (!data.empty())
		return data;

	data.resize(rorigin_size);
	for (lyx::size_type i = 0; i < rorigin_size; ++i) {
		data[i] = make_pair(_(rorigin_gui_strs[i]),
				    rorigin_lyx_strs[i]);
	}

	return data;
}

} // namespace frnt
