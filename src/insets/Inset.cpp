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
	InsetName(string const & n = string(), docstring const & dn = docstring())
		: name(n), display_name(dn) {}
	string name;
	docstring display_name;
};


static InsetName insetnames[INSET_CODE_SIZE];


// This list should be kept in sync with the list of dialogs in
// src/frontends/qt4/GuiView.cpp, I.e., if a dialog goes with an
// inset, the dialog should have the same name as the inset.
// Changes should be also recorded in LFUN_DIALOG_SHOW doxygen
// docs in LyXAction.cpp.
static void build_translator()
{
	static bool passed = false;
	if (passed)
		return;
	insetnames[TOC_CODE] = InsetName("toc");
	insetnames[QUOTE_CODE] = InsetName("quote");
	insetnames[REF_CODE] = InsetName("ref");
	insetnames[HYPERLINK_CODE] = InsetName("href");
	insetnames[SEPARATOR_CODE] = InsetName("separator");
	insetnames[ENDING_CODE] = InsetName("ending");
	insetnames[LABEL_CODE] = InsetName("label");
	insetnames[NOTE_CODE] = InsetName("note");
	insetnames[PHANTOM_CODE] = InsetName("phantom");
	insetnames[ACCENT_CODE] = InsetName("accent");
	insetnames[MATH_CODE] = InsetName("math");
	insetnames[INDEX_CODE] = InsetName("index");
	insetnames[NOMENCL_CODE] = InsetName("nomenclature");
	insetnames[INCLUDE_CODE] = InsetName("include");
	insetnames[GRAPHICS_CODE] = InsetName("graphics");
	insetnames[BIBITEM_CODE] = InsetName("bibitem", _("Bibliography Entry"));
	insetnames[BIBTEX_CODE] = InsetName("bibtex");
	insetnames[TEXT_CODE] = InsetName("text");
	insetnames[ERT_CODE] = InsetName("ert", _("TeX Code"));
	insetnames[FOOT_CODE] = InsetName("foot");
	insetnames[MARGIN_CODE] = InsetName("margin");
	insetnames[FLOAT_CODE] = InsetName("float", _("Float"));
	insetnames[WRAP_CODE] = InsetName("wrap");
	insetnames[SPECIALCHAR_CODE] = InsetName("specialchar");
	insetnames[IPA_CODE] = InsetName("ipa");
	insetnames[IPACHAR_CODE] = InsetName("ipachar");
	insetnames[IPADECO_CODE] = InsetName("ipadeco");
	insetnames[TABULAR_CODE] = InsetName("tabular", _("Table"));
	insetnames[EXTERNAL_CODE] = InsetName("external");
	insetnames[CAPTION_CODE] = InsetName("caption");
	insetnames[MATHMACRO_CODE] = InsetName("mathmacro");
	insetnames[CITE_CODE] = InsetName("citation");
	insetnames[FLOAT_LIST_CODE] = InsetName("floatlist");
	insetnames[INDEX_PRINT_CODE] = InsetName("index_print");
	insetnames[NOMENCL_PRINT_CODE] = InsetName("nomencl_print");
	insetnames[ARG_CODE] = InsetName("optarg");
	insetnames[NEWLINE_CODE] = InsetName("newline");
	insetnames[LINE_CODE] = InsetName("line");
	insetnames[BRANCH_CODE] = InsetName("branch", _("Branch"));
	insetnames[BOX_CODE] = InsetName("box", _("Box"));
	insetnames[FLEX_CODE] = InsetName("flex");
	insetnames[SPACE_CODE] = InsetName("space", _("Horizontal Space"));
	insetnames[VSPACE_CODE] = InsetName("vspace", _("Vertical Space"));
	insetnames[MATH_MACROARG_CODE] = InsetName("mathmacroarg");
	insetnames[LISTINGS_CODE] = InsetName("listings");
	insetnames[INFO_CODE] = InsetName("info", _("Info"));
	insetnames[COLLAPSABLE_CODE] = InsetName("collapsable");
	insetnames[NEWPAGE_CODE] = InsetName("newpage");
	insetnames[SCRIPT_CODE] = InsetName("script");
	insetnames[CELL_CODE] = InsetName("tablecell");
	insetnames[MATH_AMSARRAY_CODE] = InsetName("mathamsarray");
	insetnames[MATH_ARRAY_CODE] = InsetName("matharray");
	insetnames[MATH_BIG_CODE] = InsetName("mathbig");
	insetnames[MATH_BOLDSYMBOL_CODE] = InsetName("mathboldsymbol");
	insetnames[MATH_BOX_CODE] = InsetName("mathbox");
	insetnames[MATH_BRACE_CODE] = InsetName("mathbrace");
	insetnames[MATH_CANCEL_CODE] = InsetName("mathcancel");
	insetnames[MATH_CANCELTO_CODE] = InsetName("mathcancelto");
	insetnames[MATH_CASES_CODE] = InsetName("mathcases");
	insetnames[MATH_CHAR_CODE] = InsetName("mathchar");
	insetnames[MATH_COLOR_CODE] = InsetName("mathcolor");
	insetnames[MATH_COMMENT_CODE] = InsetName("mathcomment");
	insetnames[MATH_DECORATION_CODE] = InsetName("mathdecoration");
	insetnames[MATH_DELIM_CODE] = InsetName("mathdelim");
	insetnames[MATH_DIFF_CODE] = InsetName("mathdiff");
	insetnames[MATH_DOTS_CODE] = InsetName("mathdots");
	insetnames[MATH_ENSUREMATH_CODE] = InsetName("mathensuremath");
	insetnames[MATH_ENV_CODE] = InsetName("mathenv");
	insetnames[MATH_EXFUNC_CODE] = InsetName("mathexfunc");
	insetnames[MATH_EXINT_CODE] = InsetName("mathexint");
	insetnames[MATH_FONT_CODE] = InsetName("mathfont");
	insetnames[MATH_FONTOLD_CODE] = InsetName("mathfontold");
	insetnames[MATH_FRAC_CODE] = InsetName("mathfrac");
	insetnames[MATH_GRID_CODE] = InsetName("mathgrid");
	insetnames[MATH_CODE] = InsetName("math");
	insetnames[MATH_HULL_CODE] = InsetName("mathhull");
	insetnames[MATH_KERN_CODE] = InsetName("mathkern");
	insetnames[MATH_LEFTEQN_CODE] = InsetName("mathlefteqn");
	insetnames[MATH_LIM_CODE] = InsetName("mathlim");
	insetnames[MATH_MATRIX_CODE] = InsetName("mathmatrix");
	insetnames[MATH_MBOX_CODE] = InsetName("mathmbox");
	insetnames[MATH_NEST_CODE] = InsetName("mathnest");
	insetnames[MATH_NUMBER_CODE] = InsetName("mathnumber");
	insetnames[MATH_OVERSET_CODE] = InsetName("mathoverset");
	insetnames[MATH_PAR_CODE] = InsetName("mathpar");
	insetnames[MATH_PHANTOM_CODE] = InsetName("mathphantom");
	insetnames[MATH_REF_CODE] = InsetName("mathref");
	insetnames[MATH_ROOT_CODE] = InsetName("mathroot");
	insetnames[MATH_SCRIPT_CODE] = InsetName("mathscript");
	insetnames[MATH_SIZE_CODE] = InsetName("mathsize");
	insetnames[MATH_SPACE_CODE] = InsetName("mathspace", _("Horizontal Math Space"));
	insetnames[MATH_SPECIALCHAR_CODE] = InsetName("mathspecialchar");
	insetnames[MATH_SPLIT_CODE] = InsetName("mathsplit");
	insetnames[MATH_SQRT_CODE] = InsetName("mathsqrt");
	insetnames[MATH_STACKREL_CODE] = InsetName("mathstackrel");
	insetnames[MATH_STRING_CODE] = InsetName("mathstring");
	insetnames[MATH_SUBSTACK_CODE] = InsetName("mathsubstack");
	insetnames[MATH_SYMBOL_CODE] = InsetName("mathsymbol");
	insetnames[MATH_TABULAR_CODE] = InsetName("mathtabular");
	insetnames[MATH_UNDERSET_CODE] = InsetName("mathunderset");
	insetnames[MATH_UNKNOWN_CODE] = InsetName("mathunknown");
	insetnames[MATH_XARROW_CODE] = InsetName("mathxarrow");
	insetnames[MATH_XYARROW_CODE] = InsetName("mathxyarrow");
	insetnames[MATH_XYMATRIX_CODE] = InsetName("mathxymatrix");
	insetnames[MATH_DIAGRAM_CODE] = InsetName("mathdiagram");
	insetnames[MATH_MACRO_CODE] = InsetName("mathmacro");

	passed = true;
}


void Inset::setBuffer(Buffer & buffer)
{
	buffer_ = &buffer;
}


Buffer & Inset::buffer()
{
	if (!buffer_) {
		odocstringstream s;
		string const iname = insetName(lyxCode());
		LYXERR0("Inset: " << this << " LyX Code: " << lyxCode()
					<< " name: " << iname);
		s << "LyX Code: " << lyxCode() << " name: " << iname;
		LATTEST(false);
		throw ExceptionMessage(BufferException, 
			from_ascii("Inset::buffer_ member not initialized!"), s.str());
	}
	return *buffer_;
}


Buffer const & Inset::buffer() const
{
	return const_cast<Inset *>(this)->buffer();
}


bool Inset::isBufferLoaded() const
{
	return buffer_ && theBufferList().isLoaded(buffer_);
}


bool Inset::isBufferValid() const
{
	return buffer_ 
		&& (isBufferLoaded() || buffer_->isClone());
}


docstring Inset::layoutName() const
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


bool Inset::isInToc() const
{
	return getLayout().isInToc();
}


docstring Inset::toolTip(BufferView const &, int, int) const
{
	return docstring();
}


void Inset::forOutliner(docstring &, size_t const, bool const) const
{
}


string Inset::contextMenu(BufferView const &, int, int) const
{
	return contextMenuName();
}


string Inset::contextMenuName() const
{
	return string();
}


Dimension const Inset::dimension(BufferView const & bv) const
{
	return bv.coordCache().getInsets().dim(this);
}


InsetCode insetCode(string const & name)
{
	build_translator();
	for (int i = 1; i != int(INSET_CODE_SIZE); ++i) {
		if (insetnames[i].name == name)
			return InsetCode(i);
	}
	return NO_CODE;
}


string insetName(InsetCode c) 
{
	build_translator();
	return insetnames[c].name;
}


docstring insetDisplayName(InsetCode c) 
{
	build_translator();
	return insetnames[c].display_name;
}


void Inset::dispatch(Cursor & cur, FuncRequest & cmd)
{
	if (buffer_ == 0) {
		lyxerr << "Unassigned buffer_ member in Inset::dispatch()" << std::endl;
		lyxerr << "LyX Code: " << lyxCode() << " name: " 
		       << insetName(lyxCode()) << std::endl;
	} else if (cur.buffer() != buffer_)
		lyxerr << "cur.buffer() != buffer_ in Inset::dispatch()" << std::endl;
	cur.screenUpdateFlags(Update::Force | Update::FitCursor);
	cur.dispatched();
	doDispatch(cur, cmd);
}


bool Inset::showInsetDialog(BufferView * bv) const
{
	InsetCode const code = lyxCode();
	switch (code){
	case ERT_CODE:
	case FLOAT_CODE:
	case BOX_CODE:
	case BIBITEM_CODE:
	case BRANCH_CODE:
	case INFO_CODE:
	case MATH_SPACE_CODE:
	case SPACE_CODE:
	case TABULAR_CODE:
	case VSPACE_CODE:
		bv->showDialog(insetName(code));
		break;
	default:
		return false;
	}
	return true;
}


void Inset::doDispatch(Cursor & cur, FuncRequest &cmd)
{
	switch (cmd.action()) {
	case LFUN_MOUSE_RELEASE:
		// if the derived inset did not explicitly handle mouse_release,
		// we assume we request the settings dialog
		if (!cur.selection() && cmd.button() == mouse_button::button1
		    && clickable(cmd.x(), cmd.y()) && hasSettings()) {
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
		cur.noScreenUpdate();
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
	// the cursor in our inset) in lyx::getStatus().
	// Dialogs::checkStatus() ensures that the dialog is deactivated if
	// LFUN_INSET_APPLY is disabled.

	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		// Allow modification of our data.
		// This needs to be handled in the doDispatch method of our
		// instantiatable children.
		// FIXME: Why don't we let the insets determine whether this
		// should be enabled or not ? Now we need this check for 
		// the tabular features. (vfr)
		if (cmd.getArg(0) == "tabular")
			return false;
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
			return false;
		}

	case LFUN_IN_MATHMACROTEMPLATE:
		// By default we're not in a MathMacroTemplate inset
		flag.setEnabled(false);
		return true;

	case LFUN_IN_IPA:
		// By default we're not in an IPA inset
		flag.setEnabled(false);
		return true;

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
	ColorCode pen_color = mouseHovered(pi.base.bv) || editing(pi.base.bv)?
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
	ColorCode pen_color = mouseHovered(pi.base.bv) || editing(pi.base.bv)?
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
	return buffer().params().documentClass().insetLayout(layoutName());
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
	//FIXME (Abdel 03/12/10): see bugs #6814 and #6949
	// If the Buffer is null and we end up here this is most probably because we
	// are in the CutAndPaste stack. See InsetGraphics, RenderGraphics and
	// RenderPreview.
	if (!buffer_)
		return 0;
	return theApp() ? theApp()->updateInset(this) : 0;
}


bool Inset::resetFontEdit() const
{
	return getLayout().resetsFont() || !inheritFont();
}


docstring Inset::completionPrefix(Cursor const &) const 
{
	return docstring();
}

} // namespace lyx
