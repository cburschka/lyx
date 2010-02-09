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
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "MetricsInfo.h"
#include "output_xhtml.h"
#include "Text.h"
#include "TextClass.h"

#include "frontends/Application.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lassert.h"

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
		InsetName("phantom", PHANTOM_CODE),
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
		InsetName("newline", NEWLINE_CODE),
		InsetName("line", LINE_CODE),
		InsetName("branch", BRANCH_CODE),
		InsetName("box", BOX_CODE),
		InsetName("flex", FLEX_CODE),
		InsetName("space", SPACE_CODE),
		InsetName("vspace", VSPACE_CODE),
		InsetName("mathmacroarg", MATH_MACROARG_CODE),
		InsetName("listings", LISTINGS_CODE),
		InsetName("info", INFO_CODE),
		InsetName("collapsable", COLLAPSABLE_CODE),
		InsetName("newpage", NEWPAGE_CODE),
		InsetName("tablecell", CELL_CODE),
		InsetName("mathamsarray", MATH_AMSARRAY_CODE),
		InsetName("matharray", MATH_ARRAY_CODE),
		InsetName("mathbig", MATH_BIG_CODE),
		InsetName("mathboldsymbol", MATH_BOLDSYMBOL_CODE),
		InsetName("mathbox", MATH_BOX_CODE),
		InsetName("mathbrace", MATH_BRACE_CODE),
		InsetName("mathcases", MATH_CASES_CODE),
		InsetName("mathchar", MATH_CHAR_CODE),
		InsetName("mathcolor", MATH_COLOR_CODE),
		InsetName("mathcomment", MATH_COMMENT_CODE),
		InsetName("mathdecoration", MATH_DECORATION_CODE),
		InsetName("mathdelim", MATH_DELIM_CODE),
		InsetName("mathdiff", MATH_DIFF_CODE),
		InsetName("mathdots", MATH_DOTS_CODE),
		InsetName("mathensuremath", MATH_ENSUREMATH_CODE),
		InsetName("mathenv", MATH_ENV_CODE),
		InsetName("mathexfunc", MATH_EXFUNC_CODE),
		InsetName("mathexint", MATH_EXINT_CODE),
		InsetName("mathfont", MATH_FONT_CODE),
		InsetName("mathfontold", MATH_FONTOLD_CODE),
		InsetName("mathfrac", MATH_FRAC_CODE),
		InsetName("mathgrid", MATH_GRID_CODE),
		InsetName("math", MATH_CODE),
		InsetName("mathhull", MATH_HULL_CODE),
		InsetName("mathkern", MATH_KERN_CODE),
		InsetName("mathlefteqn", MATH_LEFTEQN_CODE),
		InsetName("mathlim", MATH_LIM_CODE),
		InsetName("mathmatrix", MATH_MATRIX_CODE),
		InsetName("mathmbox", MATH_MBOX_CODE),
		InsetName("mathnest", MATH_NEST_CODE),
		InsetName("mathnumber", MATH_NUMBER_CODE),
		InsetName("mathoverset", MATH_OVERSET_CODE),
		InsetName("mathpar", MATH_PAR_CODE),
		InsetName("mathphantom", MATH_PHANTOM_CODE),
		InsetName("mathref", MATH_REF_CODE),
		InsetName("mathroot", MATH_ROOT_CODE),
		InsetName("mathscript", MATH_SCRIPT_CODE),
		InsetName("mathsize", MATH_SIZE_CODE),
		InsetName("mathspace", MATH_SPACE_CODE),
		InsetName("mathspecialchar", MATH_SPECIALCHAR_CODE),
		InsetName("mathsplit", MATH_SPLIT_CODE),
		InsetName("mathsqrt", MATH_SQRT_CODE),
		InsetName("mathstackrel", MATH_STACKREL_CODE),
		InsetName("mathstring", MATH_STRING_CODE),
		InsetName("mathsubstack", MATH_SUBSTACK_CODE),
		InsetName("mathsymbol", MATH_SYMBOL_CODE),
		InsetName("mathtabular", MATH_TABULAR_CODE),
		InsetName("mathunderset", MATH_UNDERSET_CODE),
		InsetName("mathunknown", MATH_UNKNOWN_CODE),
		InsetName("mathxarrow", MATH_XARROW_CODE),
		InsetName("mathxyarrow", MATH_XYARROW_CODE),
		InsetName("mathxymatrix", MATH_XYMATRIX_CODE),
		InsetName("mathmacro", MATH_MACRO_CODE),
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
		lyxerr << "Inset: " << this
					<< " LyX Code: " << lyxCode()
					<< " name: " << insetName(lyxCode())
					<< std::endl;
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


bool Inset::isBufferValid() const
{
	return buffer_ && theBufferList().isLoaded(buffer_);
}


docstring Inset::name() const
{
	return from_ascii("unknown");
}


bool Inset::isFreeSpacing() const 
{
	return getLayout().isFreeSpacing();
}


bool Inset::allowEmpty() const
{
	return getLayout().isKeepEmpty();
}

bool Inset::forceLTR() const
{
	return getLayout().forceLTR();
}

void Inset::initView()
{
	if (isLabeled())
		buffer().updateLabels();
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
	if (buffer_ == 0) {
		lyxerr << "Unassigned buffer_ member in Inset::dispatch()" << std::endl;
		lyxerr << "LyX Code: " << lyxCode() << " name: " << insetName(lyxCode()) << std::endl;
	} else if (cur.buffer() != buffer_)
		lyxerr << "cur.buffer() != buffer_ in Inset::dispatch()" << std::endl;
	cur.updateFlags(Update::Force | Update::FitCursor);
	cur.dispatched();
	doDispatch(cur, cmd);
}


void Inset::doDispatch(Cursor & cur, FuncRequest &cmd)
{
	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
		// if the derived inset did not explicitly handle mouse_release,
		// we assume we request the settings dialog
		if (!cur.selection() && cmd.button() == mouse_button::button1
			  && hasSettings()) {
			FuncRequest tmpcmd(LFUN_INSET_SETTINGS);
			dispatch(cur, tmpcmd);
		}
		break;

	case LFUN_INSET_SETTINGS:
		if (cmd.argument().empty() || cmd.getArg(0) == insetName(lyxCode())) {
			showInsetDialog(&cur.bv());
			cur.dispatched();
		} else
			cur.undispatched();
		break;

	default:
		cur.noUpdate();
		cur.undispatched();
		break;
	}
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
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_INSERT:
		// Don't allow insertion of new insets.
		// Every inset that wants to allow new insets from open
		// dialogs needs to override this.
		flag.setEnabled(false);
		return true;

	case LFUN_INSET_SETTINGS:
		if (cmd.argument().empty() || cmd.getArg(0) == insetName(lyxCode())) {
			bool const enable = hasSettings();
			flag.setEnabled(enable);
			return true;
		} else {
			flag.setEnabled(false);
			return false;
		}

	default:
		break;
	}
	return false;
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


docstring Inset::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << "[[Inset: " << from_ascii(insetName(lyxCode())) << "]]";
	return docstring();
}


bool Inset::directWrite() const
{
	return false;
}


bool Inset::editable() const
{
	return false;
}


bool Inset::hasSettings() const
{
	return false;
}



bool Inset::autoDelete() const
{
	return false;
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


InsetLayout const & Inset::getLayout() const
{
	if (!buffer_)
		return DocumentClass::plainInsetLayout();
	return buffer().params().documentClass().insetLayout(name());
}


bool Inset::undefined() const
{
	docstring const & n = getLayout().name();
	return n.empty() || n == DocumentClass::plainInsetLayout().name();
}


void Inset::dump() const
{
	write(lyxerr);
}


ColorCode Inset::backgroundColor(PainterInfo const & /*pi*/) const
{
	return Color_none;
}


ColorCode Inset::labelColor() const
{
	return Color_foreground;
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
