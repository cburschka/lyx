/**
 * \file ControlGraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlGraphics.h"

#include "helper_funcs.h"

#include "funcrequest.h"
#include "gettext.h"
#include "lyxrc.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "insets/insetgraphics.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/globbing.h"
#include "support/path_defines.h"
#include "support/tostr.h"
#include "support/types.h"

using std::make_pair;
using std::string;
using std::pair;
using std::vector;

namespace lyx {

using support::AddName;
using support::FileFilterList;
using support::FileInfo;
using support::IsFileReadable;
using support::MakeAbsPath;
using support::readBB_from_PSFile;
using support::system_lyxdir;
using support::user_lyxdir;

namespace frontend {


ControlGraphics::ControlGraphics(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlGraphics::initialiseParams(string const & data)
{
	InsetGraphicsParams params;
	InsetGraphicsMailer::string2params(data, kernel().buffer(), params);
	params_.reset(new InsetGraphicsParams(params));
	return true;
}


void ControlGraphics::clearParams()
{
	params_.reset();
}


void ControlGraphics::dispatchParams()
{
	InsetGraphicsParams tmp_params(params());
	string const lfun =
		InsetGraphicsMailer::params2string(tmp_params, kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


string const ControlGraphics::browse(string const & in_name) const
{
	string const title = _("Select graphics file");

	// Does user clipart directory exist?
	string clipdir = AddName (user_lyxdir(), "clipart");
	FileInfo fileInfo(clipdir);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		clipdir = AddName (system_lyxdir(), "clipart");
	pair<string, string> dir1(_("Clipart|#C#c"), clipdir);
	pair<string, string> dir2(_("Documents|#o#O"), string(lyxrc.document_path));
	// Show the file browser dialog
	return browseRelFile(in_name, kernel().bufferFilepath(),
			     title,
			     FileFilterList(),
			     false, dir1, dir2);
}


string const ControlGraphics::readBB(string const & file)
{
	string const abs_file =
		MakeAbsPath(file, kernel().bufferFilepath());

	// try to get it from the file, if possible. Zipped files are
	// unzipped in the readBB_from_PSFile-Function
	string const bb = readBB_from_PSFile(abs_file);
	if (!bb.empty())
		return bb;

	// we don't, so ask the Graphics Cache if it has loaded the file
	int width = 0;
	int height = 0;

	graphics::Cache & gc = graphics::Cache::get();
	if (gc.inCache(abs_file)) {
		graphics::Image const * image = gc.item(abs_file)->image();

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
	string const name = MakeAbsPath(fname, kernel().bufferFilepath());
	return IsFileReadable(name);
}


void ControlGraphics::editGraphics()
{
	BOOST_ASSERT(params_.get());

	dialog().view().apply();
	string const lfun =
		InsetGraphicsMailer::params2string(params(), kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_GRAPHICS_EDIT, lfun));
}


namespace {

char const * const bb_units[] = { "bp", "cm", "mm", "in" };
size_t const bb_size = sizeof(bb_units) / sizeof(char *);

// These are the strings that are stored in the LyX file and which
// correspond to the LaTeX identifiers shown in the comments at the
// end of each line.
char const * const rorigin_lyx_strs[] = {
	// the LaTeX default is leftBaseline
	"",
	"leftTop",  "leftBottom", "leftBaseline", // lt lb lB
	"center", "centerTop", "centerBottom", "centerBaseline", // c ct cb cB
	"rightTop", "rightBottom", "rightBaseline" }; // rt rb rB

// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const rorigin_gui_strs[] = {
	N_("Default"),
	N_("Top left"), N_("Bottom left"), N_("Baseline left"),
	N_("Center"), N_("Top center"), N_("Bottom center"), N_("Baseline center"),
	N_("Top right"), N_("Bottom right"), N_("Baseline right") };

size_t const rorigin_size = sizeof(rorigin_lyx_strs) / sizeof(char *);

} // namespace anon


vector<string> const getBBUnits()
{
	return vector<string> (bb_units, bb_units + bb_size);
}


vector<RotationOriginPair> getRotationOriginData()
{
	static vector<RotationOriginPair> data;
	if (!data.empty())
		return data;

	data.resize(rorigin_size);
	for (size_type i = 0; i < rorigin_size; ++i) {
		data[i] = make_pair(_(rorigin_gui_strs[i]),
				    rorigin_lyx_strs[i]);
	}

	return data;
}

} // namespace frontend
} // namespace lyx
