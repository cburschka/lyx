/**
 * \file ControlExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlExternal.h"

#include "funcrequest.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "lyxrc.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "insets/insetexternal.h"
#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"

#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/tostr.h"

using std::advance;
using std::vector;
using std::string;

namespace lyx {

using support::FileFilterList;
using support::MakeAbsPath;
using support::readBB_from_PSFile;

namespace frontend {


ControlExternal::ControlExternal(Dialog & parent)
	: Dialog::Controller(parent),
	  bb_changed_(false)
{}


bool ControlExternal::initialiseParams(string const & data)
{
	params_.reset(new InsetExternalParams);
	InsetExternalMailer::string2params(data, kernel().buffer(), *params_);
	return true;
}


void ControlExternal::clearParams()
{
	params_.reset();
}


void ControlExternal::dispatchParams()
{
	string const lfun = InsetExternalMailer::params2string(params(),
							       kernel().buffer());

	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void ControlExternal::setParams(InsetExternalParams const & p)
{
	BOOST_ASSERT(params_.get());
	*params_ = p;
}


InsetExternalParams const & ControlExternal::params() const
{
	BOOST_ASSERT(params_.get());
	return *params_;
}


void ControlExternal::editExternal()
{
	BOOST_ASSERT(params_.get());

	dialog().view().apply();
	string const lfun =
		InsetExternalMailer::params2string(params(), kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_EXTERNAL_EDIT, lfun));
}


vector<string> const ControlExternal::getTemplates() const
{
	vector<string> result;

	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();

	for (; i1 != i2; ++i1) {
		result.push_back(i1->second.lyxName);
	}
	return result;
}


int ControlExternal::getTemplateNumber(string const & name) const
{
	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();
	for (int i = 0; i1 != i2; ++i1, ++i) {
		if (i1->second.lyxName == name)
			return i;
	}

	// we can get here if a LyX document has a template not installed
	// on this machine.
	return -1;
}


external::Template ControlExternal::getTemplate(int i) const
{
	external::TemplateManager::Templates::const_iterator i1
		= external::TemplateManager::get().getTemplates().begin();

	advance(i1, i);

	return i1->second;
}


string const ControlExternal::browse(string const & input,
				     string const & template_name) const
{
	string const title =  _("Select external file");

	string const bufpath = kernel().bufferFilepath();

	/// Determine the template file extension
	external::TemplateManager const & etm =
		external::TemplateManager::get();
	external::Template const * const et_ptr =
		etm.getTemplateByName(template_name);

	FileFilterList const filter = et_ptr ?
		FileFilterList(et_ptr->fileRegExp) :
		FileFilterList();

	std::pair<string, string> dir1(N_("Documents|#o#O"),
				       string(lyxrc.document_path));

	return browseRelFile(input, bufpath, title, filter, false, dir1);
}


string const ControlExternal::readBB(string const & file)
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

} // namespace frontend


namespace external {

namespace {

RotationDataType origins_array[] = {
	RotationData::DEFAULT,
	RotationData::TOPLEFT,
	RotationData::BOTTOMLEFT,
	RotationData::BASELINELEFT,
	RotationData::CENTER,
	RotationData::TOPCENTER,
	RotationData::BOTTOMCENTER,
	RotationData::BASELINECENTER,
	RotationData::TOPRIGHT,
	RotationData::BOTTOMRIGHT,
	RotationData::BASELINERIGHT
};


size_type const origins_array_size =
sizeof(origins_array) / sizeof(origins_array[0]);

vector<RotationDataType> const
origins(origins_array, origins_array + origins_array_size);

// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const origin_gui_strs[] = {
	N_("Default"),
	N_("Top left"), N_("Bottom left"), N_("Baseline left"),
	N_("Center"), N_("Top center"), N_("Bottom center"), N_("Baseline center"),
	N_("Top right"), N_("Bottom right"), N_("Baseline right")
};

} // namespace anon


vector<RotationDataType> const & all_origins()
{
	return origins;
}

string const origin_gui_str(size_type i)
{
	return origin_gui_strs[i];
}

} // namespace external
} // namespace lyx
