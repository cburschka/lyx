/**
 * \file OutputParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "OutputParams.h"
#include "Exporter.h"
#include "Encoding.h"


namespace lyx {


OutputParams::OutputParams(Encoding const * enc)
	: encoding(enc), exportdata(new ExportData)
{
	// Note: in PreviewLoader::Impl::dumpPreamble
	// OutputParams runparams(0);
	if (enc && enc->package() == Encoding::japanese)
		use_japanese = true;
	if (enc && enc->package() == Encoding::CJK)
		use_CJK = true;
}


OutputParams::~OutputParams()
{}


bool OutputParams::isLaTeX() const
{
	return flavor == Flavor::LaTeX
			|| flavor == Flavor::LuaTeX
			|| flavor == Flavor::DviLuaTeX
			|| flavor == Flavor::PdfLaTeX
			|| flavor == Flavor::XeTeX;
}


bool OutputParams::isFullUnicode() const
{
	return flavor == Flavor::LuaTeX
			|| flavor == Flavor::DviLuaTeX
			|| flavor == Flavor::XeTeX;
}


bool OutputParams::useBidiPackage() const
{
	return use_polyglossia && flavor == Flavor::XeTeX;
}

} // namespace lyx
