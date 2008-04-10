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

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "MetricsInfo.h"
#include "Text.h"
#include "TextClass.h"

#include "frontends/Painter.h"
#include "frontends/Application.h"

#include "support/assert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"

#include <map>

using namespace std;
using namespace lyx::support;

namespace lyx {

class InsetName {
public:
	InsetName(string const & n, InsetCode c) : name(n), code(c) {}
	string name;
	InsetCode code;
};


typedef map<string, InsetCode> TranslatorMap;


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
		InsetName("newline", NEWLINE_CODE),
		InsetName("line", LINE_CODE),
		InsetName("branch", BRANCH_CODE),
		InsetName("box", BOX_CODE),
		InsetName("flex", FLEX_CODE),
		InsetName("space", SPACE_CODE),
		InsetName("vspace", VSPACE_CODE),
		InsetName("mathmacroarg", MATHMACROARG_CODE),
		InsetName("listings", LISTINGS_CODE),
		InsetName("info", INFO_CODE),
		InsetName("collapsable", COLLAPSABLE_CODE),
		InsetName("newpage", NEWPAGE_CODE),
		InsetName("tablecell", CELL_CODE)
	};

	size_t const insetnames_size =
		sizeof(insetnames) / sizeof(insetnames[0]);

	map<string, InsetCode> data;
	for (size_t i = 0; i != insetnames_size; ++i) {
		InsetName const & var = insetnames[i];
		data[var.name] = var.code;
	}

	return data;
}


void Inset::setBuffer(Buffer & buffer)
{
	buffer_ = &buffer;
}


Buffer & Inset::buffer()
{
	if (!buffer_) {
		odocstringstream s;
		lyxerr << "LyX Code: " << lyxCode() << " name: " << name() << std::endl;
		s << "LyX Code: " << lyxCode() << " name: " << name();
		LASSERT(false, /**/);
		throw ExceptionMessage(BufferException, 
			from_ascii("Inset::buffer_ member not initialized!"), s.str());
	}
	return *buffer_;
}


Buffer const & Inset::buffer() const
{
	return const_cast<Inset *>(this)->buffer();
}


docstring Inset::name() const
{
	return from_ascii("unknown");
}


void Inset::initView()
{
	if (isLabeled())
		lyx::updateLabels(buffer());
}


docstring Inset::toolTip(BufferView const &, int, int) const
{
	return docstring();
}


docstring Inset::contextMenu(BufferView const &, int, int) const
{
	return docstring();
}


Dimension const Inset::dimension(BufferView const & bv) const
{
	return bv.coordCache().getInsets().dim(this);
}


InsetCode insetCode(string const & name)
{
	static TranslatorMap const translator = build_translator();

	TranslatorMap::const_iterator it = translator.find(name);
	return it == translator.end() ? NO_CODE : it->second;
}


string insetName(InsetCode c) 
{
	static TranslatorMap const translator = build_translator();

	TranslatorMap::const_iterator it =  translator.begin();
	TranslatorMap::const_iterator end = translator.end();
	for (; it != end; ++it) {
		if (it->second == c)
			return it->first;
	}
	return string();
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


void Inset::edit(Cursor &, bool, EntryDirection)
{
	LYXERR(Debug::INSETS, "edit left/right");
}


Inset * Inset::editXY(Cursor &, int x, int y)
{
	LYXERR(Debug::INSETS, "x: " << x << " y: " << y);
	return this;
}


Inset::idx_type Inset::index(row_type row, col_type col) const
{
	if (row != 0)
		LYXERR0("illegal row: " << row);
	if (col != 0)
		LYXERR0("illegal col: " << col);
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


int Inset::docbook(odocstream &, OutputParams const &) const
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


docstring Inset::editMessage() const
{
	return _("Opened inset");
}


void Inset::cursorPos(BufferView const & /*bv*/, CursorSlice const &,
		bool, int & x, int & y) const
{
	LYXERR0("Inset::cursorPos called directly");
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
	ColorCode pen_color = mouseHovered() || editing(pi.base.bv)?
		Color_mathframe : Color_mathcorners;

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
	ColorCode pen_color = mouseHovered() || editing(pi.base.bv)?
		Color_mathframe : Color_mathcorners;

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


bool Inset::editing(BufferView const * bv) const
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
	return bp.documentClass().insetLayout(name());  
}


void Inset::dump() const
{
	write(lyxerr);
}


ColorCode Inset::backgroundColor() const
{
	return Color_background;
}


void Inset::setPosCache(PainterInfo const & pi, int x, int y) const
{
	//LYXERR("Inset: set position cache to " << x << " " << y);
	pi.base.bv->coordCache().insets().add(this, x, y);
}


void Inset::setDimCache(MetricsInfo const & mi, Dimension const & dim) const
{
	mi.base.bv->coordCache().insets().add(this, dim);
}


Buffer const * Inset::updateFrontend() const
{
	return theApp() ? theApp()->updateInset(this) : 0;
}


docstring Inset::completionPrefix(Cursor const &) const 
{
	return docstring();
}

} // namespace lyx
