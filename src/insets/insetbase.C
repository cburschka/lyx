/**
 * \file insetbase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbase.h"

#include "buffer.h"
#include "coordcache.h"
#include "BufferView.h"
#include "LColor.h"
#include "cursor.h"
#include "debug.h"
#include "dimension.h"
#include "dispatchresult.h"
#include "gettext.h"
#include "lyxtext.h"
#include "metricsinfo.h"

#include "frontends/Painter.h"

#include <map>


namespace {

struct InsetName {
	InsetName(std::string const & n, InsetBase::Code c)
		: name(n), code(c) {}
	std::string name;
	InsetBase::Code code;
};


typedef std::map<std::string, InsetBase::Code> TranslatorMap;


TranslatorMap const build_translator()
{
	InsetName const insetnames[] = {
		InsetName("toc", InsetBase::TOC_CODE),
		InsetName("quote", InsetBase::QUOTE_CODE),
		InsetName("ref", InsetBase::REF_CODE),
		InsetName("url", InsetBase::URL_CODE),
		InsetName("htmlurl", InsetBase::HTMLURL_CODE),
		InsetName("separator", InsetBase::SEPARATOR_CODE),
		InsetName("ending", InsetBase::ENDING_CODE),
		InsetName("label", InsetBase::LABEL_CODE),
		InsetName("note", InsetBase::NOTE_CODE),
		InsetName("accent", InsetBase::ACCENT_CODE),
		InsetName("math", InsetBase::MATH_CODE),
		InsetName("index", InsetBase::INDEX_CODE),
		InsetName("include", InsetBase::INCLUDE_CODE),
		InsetName("graphics", InsetBase::GRAPHICS_CODE),
		InsetName("bibitem", InsetBase::BIBITEM_CODE),
		InsetName("bibtex", InsetBase::BIBTEX_CODE),
		InsetName("text", InsetBase::TEXT_CODE),
		InsetName("ert", InsetBase::ERT_CODE),
		InsetName("foot", InsetBase::FOOT_CODE),
		InsetName("margin", InsetBase::MARGIN_CODE),
		InsetName("float", InsetBase::FLOAT_CODE),
		InsetName("wrap", InsetBase::WRAP_CODE),
		InsetName("specialchar", InsetBase::SPECIALCHAR_CODE),
		InsetName("tabular", InsetBase::TABULAR_CODE),
		InsetName("external", InsetBase::EXTERNAL_CODE),
		InsetName("caption", InsetBase::CAPTION_CODE),
		InsetName("mathmacro", InsetBase::MATHMACRO_CODE),
		InsetName("error", InsetBase::ERROR_CODE),
		InsetName("cite", InsetBase::CITE_CODE),
		InsetName("float_list", InsetBase::FLOAT_LIST_CODE),
		InsetName("index_print", InsetBase::INDEX_PRINT_CODE),
		InsetName("optarg", InsetBase::OPTARG_CODE),
		InsetName("environment", InsetBase::ENVIRONMENT_CODE),
		InsetName("hfill", InsetBase::HFILL_CODE),
		InsetName("newline", InsetBase::NEWLINE_CODE),
		InsetName("line", InsetBase::LINE_CODE),
		InsetName("branch", InsetBase::BRANCH_CODE),
		InsetName("box", InsetBase::BOX_CODE),
		InsetName("charstyle", InsetBase::CHARSTYLE_CODE),
		InsetName("vspace", InsetBase::VSPACE_CODE),
		InsetName("mathmacroarg", InsetBase::MATHMACROARG_CODE),
	};

	std::size_t const insetnames_size =
		sizeof(insetnames) / sizeof(insetnames[0]);

	std::map<std::string, InsetBase::Code> data;
	for (std::size_t i = 0; i != insetnames_size; ++i) {
		InsetName const & var = insetnames[i];
		data[var.name] = var.code;
	}

	return data;
}

} // namespace anon


InsetBase::InsetBase()
{}


InsetBase::InsetBase(InsetBase const &)
{}


InsetBase::Code InsetBase::translate(std::string const & name)
{
	static TranslatorMap const translator = build_translator();

	TranslatorMap::const_iterator it = translator.find(name);
	return it == translator.end() ? NO_CODE : it->second;
}


void InsetBase::dispatch(LCursor & cur, FuncRequest & cmd)
{
	cur.needsUpdate();
	cur.dispatched();
	priv_dispatch(cur, cmd);
}


void InsetBase::priv_dispatch(LCursor & cur, FuncRequest &)
{
	cur.noUpdate();
	cur.undispatched();
}


bool InsetBase::getStatus(LCursor &, FuncRequest const &, FuncStatus &) const
{
	return false;
}


void InsetBase::edit(LCursor &, bool)
{
	lyxerr << "InsetBase: edit left/right" << std::endl;
}


InsetBase * InsetBase::editXY(LCursor &, int x, int y) const
{
	lyxerr << "InsetBase: editXY x:" << x << " y: " << y << std::endl;
	return const_cast<InsetBase*>(this);
}


InsetBase::idx_type InsetBase::index(row_type row, col_type col) const
{
	if (row != 0)
		lyxerr << "illegal row: " << row << std::endl;
	if (col != 0)
		lyxerr << "illegal col: " << col << std::endl;
	return 0;
}


bool InsetBase::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	return from <= idx && idx <= to;
}


bool InsetBase::idxUpDown(LCursor &, bool) const
{
	return false;
}


bool InsetBase::idxUpDown2(LCursor &, bool) const
{
	return false;
}


int InsetBase::plaintext(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


int InsetBase::linuxdoc(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


int InsetBase::docbook(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


bool InsetBase::directWrite() const
{
	return false;
}


InsetBase::EDITABLE InsetBase::editable() const
{
	return NOT_EDITABLE;
}


bool InsetBase::autoDelete() const
{
	return false;
}


std::string const InsetBase::editMessage() const
{
	return _("Opened inset");
}


std::string const & InsetBase::getInsetName() const
{
	static std::string const name = "unknown";
	return name;
}


void InsetBase::markErased()
{}


void InsetBase::getCursorPos(LCursor const &, int & x, int & y) const
{
	lyxerr << "InsetBase::getCursorPos called directly" << std::endl;
	x = 100;
	y = 100;
}


void InsetBase::metricsMarkers(Dimension & dim, int framesize) const
{
	dim.wid += 2 * framesize;
	dim.asc += framesize;
}


void InsetBase::metricsMarkers2(Dimension & dim, int framesize) const
{
	dim.wid += 2 * framesize;
	dim.asc += framesize;
	dim.des += framesize;
}


void InsetBase::drawMarkers(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	int const t = x + width() - 1;
	int const d = y + descent();
	pi.pain.line(x, d - 3, x, d, LColor::mathframe);
	pi.pain.line(t, d - 3, t, d, LColor::mathframe);
	pi.pain.line(x, d, x + 3, d, LColor::mathframe);
	pi.pain.line(t - 3, d, t, d, LColor::mathframe);
	setPosCache(pi, x, y);
}


void InsetBase::drawMarkers2(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	drawMarkers(pi, x, y);
	int const t = x + width() - 1;
	int const a = y - ascent();
	pi.pain.line(x, a + 3, x, a, LColor::mathframe);
	pi.pain.line(t, a + 3, t, a, LColor::mathframe);
	pi.pain.line(x, a, x + 3, a, LColor::mathframe);
	pi.pain.line(t - 3, a, t, a, LColor::mathframe);
	setPosCache(pi, x, y);
}


bool InsetBase::editing(BufferView * bv) const
{
	return bv->cursor().isInside(this);
}


int InsetBase::xo() const
{
	return theCoords.insets_.x(this);
}


int InsetBase::yo() const
{
	return theCoords.insets_.y(this);
}


bool InsetBase::covers(int x, int y) const
{
	//lyxerr << "InsetBase::covers, x: " << x << " y: " << y
	//	<< " xo: " << xo() << " yo: " << yo()
	//	<< " x1: " << xo() << " x2: " << xo() + width()
	//	<< " y1: " << yo() - ascent() << " y2: " << yo() + descent()
	//	<< std::endl;
	return theCoords.insets_.has(this)
			&& x >= xo()
			&& x <= xo() + width()
			&& y >= yo() - ascent()
			&& y <= yo() + descent();
}


void InsetBase::dump() const
{
	Buffer buf("foo", 1);
	write(buf, lyxerr);
}


/////////////////////////////////////////

bool isEditableInset(InsetBase const * inset)
{
	return inset && inset->editable();
}


bool isHighlyEditableInset(InsetBase const * inset)
{
	return inset && inset->editable() == InsetBase::HIGHLY_EDITABLE;
}
