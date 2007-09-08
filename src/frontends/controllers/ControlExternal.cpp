/**
 * \file ControlExternal.cpp
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

#include "FuncRequest.h"
#include "gettext.h"
#include "frontend_helpers.h"
#include "LyXRC.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "insets/InsetExternal.h"
#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"

#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/convert.h"

using std::advance;
using std::vector;
using std::string;

namespace lyx {

using support::FileFilterList;
using support::FileName;
using support::makeAbsPath;
using support::readBB_from_PSFile;

namespace frontend {


ControlExternal::ControlExternal(Dialog & parent)
	: Controller(parent),
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

	kernel().dispatch(FuncRequest(getLfun(), lfun));
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

	dialog().applyView();
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


string const
ControlExternal::getTemplateFilters(string const & template_name) const
{
	/// Determine the template file extension
	external::TemplateManager const & etm =
		external::TemplateManager::get();
	external::Template const * const et_ptr =
		etm.getTemplateByName(template_name);

	if (et_ptr)
		return et_ptr->fileRegExp;

	return string();
}


docstring const ControlExternal::browse(docstring const & input,
				     docstring const & template_name) const
{
	docstring const title =  _("Select external file");

	docstring const bufpath = lyx::from_utf8(kernel().bufferFilepath());
	FileFilterList const filter =
		FileFilterList(lyx::from_utf8(getTemplateFilters(lyx::to_utf8(template_name))));

	std::pair<docstring, docstring> dir1(_("Documents|#o#O"),
		lyx::from_utf8(lyxrc.document_path));

	return browseRelFile(input, bufpath, title, filter, false, dir1);
}


string const ControlExternal::readBB(string const & file)
{
	FileName const abs_file(makeAbsPath(file, kernel().bufferFilepath()));

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

	return ("0 0 " + convert<string>(width) + ' ' + convert<string>(height));
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

docstring const origin_gui_str(size_type i)
{
	return _(origin_gui_strs[i]);
}

} // namespace external
} // namespace lyx
