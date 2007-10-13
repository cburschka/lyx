/**
 * \file Inset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Inset.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Color.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "debug.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Text.h"
#include "TextClass.h"
#include "MetricsInfo.h"
#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include "support/convert.h"

#include <boost/current_function.hpp>

#include <map>
#include <typeinfo>


namespace lyx {

class InsetName {
public:
	InsetName(std::string const & n, InsetCode c)
		: name(n), code(c) {}
	std::string name;
	InsetCode code;
};


typedef std::map<std::string, InsetCode> TranslatorMap;


static TranslatorMap const build_translator()
{
	InsetName const insetnames[] = {
		InsetName("toc", TOC_CODE),
		InsetName("quote", QUOTE_CODE),
		InsetName("ref", REF_CODE),
		InsetName("href", HYPERLINK_CODE),
		InsetName("separator", SEPARATOR_CODE),
		InsetName("ending", ENDING_CODE),
		InsetName("label", LABEL_CODE),
		InsetName("note", NOTE_CODE),
		InsetName("accent", ACCENT_CODE),
		InsetName("math", MATH_CODE),
		InsetName("index", INDEX_CODE),
		InsetName("nomenclature", NOMENCL_CODE),
		InsetName("include", INCLUDE_CODE),
		InsetName("graphics", GRAPHICS_CODE),
		InsetName("bibitem", BIBITEM_CODE),
		InsetName("bibtex", BIBTEX_CODE),
		InsetName("text", TEXT_CODE),
		InsetName("ert", ERT_CODE),
		InsetName("foot", FOOT_CODE),
		InsetName("margin", MARGIN_CODE),
		InsetName("float", FLOAT_CODE),
		InsetName("wrap", WRAP_CODE),
		InsetName("specialchar", SPECIALCHAR_CODE),
		InsetName("tabular", TABULAR_CODE),
		InsetName("external", EXTERNAL_CODE),
		InsetName("caption", CAPTION_CODE),
		InsetName("mathmacro", MATHMACRO_CODE),
		InsetName("citation", CITE_CODE),
		InsetName("floatlist", FLOAT_LIST_CODE),
		InsetName("index_print", INDEX_PRINT_CODE),
		InsetName("nomencl_print", NOMENCL_PRINT_CODE),
		InsetName("optarg", OPTARG_CODE),
		InsetName("environment", ENVIRONMENT_CODE),
		InsetName("hfill", HFILL_CODE),
		InsetName("newline", NEWLINE_CODE),
		InsetName("line", LINE_CODE),
		InsetName("branch", BRANCH_CODE),
		InsetName("box", BOX_CODE),
		InsetName("flex", FLEX_CODE),
		InsetName("vspace", VSPACE_CODE),
		InsetName("mathmacroarg", MATHMACROARG_CODE),
		InsetName("listings", LISTINGS_CODE),
		InsetName("info", INFO_CODE),
	};

	std::size_t const insetnames_size =
		sizeof(insetnames) / sizeof(insetnames[0]);

	std::map<std::string, InsetCode> data;
	for (std::size_t i = 0; i != insetnames_size; ++i) {
		InsetName const & var = insetnames[i];
		data[var.name] = var.code;
	}

	return data;
}


Inset::Inset()
{}


Dimension const Inset::dimension(BufferView const & bv) const
{
	return bv.coordCache().getInsets().dim(this);
}


InsetCode insetCode(std::string const & name)
{
	static TranslatorMap const translator = build_translator();

	TranslatorMap::const_iterator it = translator.find(name);
	return it == translator.end() ? NO_CODE : it->second;
}


void Inset::dispatch(Cursor & cur, FuncRequest & cmd)
{
	cur.updateFlags(Update::Force | Update::FitCursor);
	cur.dispatched();
	doDispatch(cur, cmd);
}


void Inset::doDispatch(Cursor & cur, FuncRequest &)
{
	cur.noUpdate();
	cur.undispatched();
}


bool Inset::getStatus(Cursor &, FuncRequest const & cmd,
	FuncStatus & flag) const
{
	// LFUN_INSET_APPLY is sent from the dialogs when the data should
	// be applied. This is either changed to LFUN_INSET_MODIFY (if the
	// dialog belongs to us) or LFUN_INSET_INSERT (if the dialog does
	// not belong to us, i. e. the dialog was open, and the user moved
	// the cursor in our inset) in LyXFunc::getStatus().
	// Dialogs::checkStatus() ensures that the dialog is deactivated if
	// LFUN_INSET_APPLY is disabled.

	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
		// Allow modification of our data.
		// This needs to be handled in the doDispatch method of our
		// instantiatable children.
		flag.enabled(true);
		return true;

	case LFUN_INSET_INSERT:
		// Don't allow insertion of new insets.
		// Every inset that wants to allow new insets from open
		// dialogs needs to override this.
		flag.enabled(false);
		return true;

	default:
		return false;
	}
}


void Inset::edit(Cursor &, bool)
{
	LYXERR(Debug::INSETS) << BOOST_CURRENT_FUNCTION
			      << ": edit left/right" << std::endl;
}


Inset * Inset::editXY(Cursor &, int x, int y)
{
	LYXERR(Debug::INSETS) << BOOST_CURRENT_FUNCTION
			      << ": x=" << x << " y= " << y
			      << std::endl;
	return this;
}


Inset::idx_type Inset::index(row_type row, col_type col) const
{
	if (row != 0)
		lyxerr << BOOST_CURRENT_FUNCTION
		       << ": illegal row: " << row << std::endl;
	if (col != 0)
		lyxerr << BOOST_CURRENT_FUNCTION
		       << ": illegal col: " << col << std::endl;
	return 0;
}


bool Inset::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	return from <= idx && idx <= to;
}


bool Inset::idxUpDown(Cursor &, bool) const
{
	return false;
}


int Inset::docbook(Buffer const &,
	odocstream &, OutputParams const &) const
{
	return 0;
}


bool Inset::directWrite() const
{
	return false;
}


Inset::EDITABLE Inset::editable() const
{
	return NOT_EDITABLE;
}


bool Inset::autoDelete() const
{
	return false;
}


docstring const Inset::editMessage() const
{
	return _("Opened inset");
}


void Inset::cursorPos(BufferView const & /*bv*/, CursorSlice const &,
		bool, int & x, int & y) const
{
	lyxerr << "Inset::cursorPos called directly" << std::endl;
	x = 100;
	y = 100;
}


void Inset::metricsMarkers(Dimension & dim, int framesize) const
{
	dim.wid += 2 * framesize;
	dim.des += framesize;
}


void Inset::metricsMarkers2(Dimension & dim, int framesize) const
{
	dim.wid += 2 * framesize;
	dim.asc += framesize;
	dim.des += framesize;
}


void Inset::drawMarkers(PainterInfo & pi, int x, int y) const
{
	Color::color pen_color = mouseHovered() || editing(pi.base.bv)?
		Color::mathframe : Color::mathcorners;

	Dimension const dim = dimension(*pi.base.bv);

	int const t = x + dim.width() - 1;
	int const d = y + dim.descent();
	pi.pain.line(x, d - 3, x, d, pen_color);
	pi.pain.line(t, d - 3, t, d, pen_color);
	pi.pain.line(x, d, x + 3, d, pen_color);
	pi.pain.line(t - 3, d, t, d, pen_color);
	setPosCache(pi, x, y);
}


void Inset::drawMarkers2(PainterInfo & pi, int x, int y) const
{
	Color::color pen_color = mouseHovered() || editing(pi.base.bv)?
		Color::mathframe : Color::mathcorners;

	drawMarkers(pi, x, y);
	Dimension const dim = dimension(*pi.base.bv);
	int const t = x + dim.width() - 1;
	int const a = y - dim.ascent();
	pi.pain.line(x, a + 3, x, a, pen_color);
	pi.pain.line(t, a + 3, t, a, pen_color);
	pi.pain.line(x, a, x + 3, a, pen_color);
	pi.pain.line(t - 3, a, t, a, pen_color);
	setPosCache(pi, x, y);
}


bool Inset::editing(BufferView * bv) const
{
	return bv->cursor().isInside(this);
}


int Inset::xo(BufferView const & bv) const
{
	return bv.coordCache().getInsets().x(this);
}


int Inset::yo(BufferView const & bv) const
{
	return bv.coordCache().getInsets().y(this);
}


bool Inset::covers(BufferView const & bv, int x, int y) const
{
	return bv.coordCache().getInsets().covers(this, x, y);
}


InsetLayout const & Inset::getLayout(BufferParams const & bp) const
{
	return bp.getTextClass().insetlayout(name());  
}


void Inset::dump() const
{
	Buffer buf("foo", 1);
	write(buf, lyxerr);
}


Color_color Inset::backgroundColor() const
{
	return Color::background;
}


void Inset::setPosCache(PainterInfo const & pi, int x, int y) const
{
	//lyxerr << "Inset:: position cache to " << x << " " << y << std::endl;
	pi.base.bv->coordCache().insets().add(this, x, y);
}


void Inset::setDimCache(MetricsInfo const & mi, Dimension const & dim) const
{
	mi.base.bv->coordCache().insets().add(this, dim);
}

} // namespace lyx
