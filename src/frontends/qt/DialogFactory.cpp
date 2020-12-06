/**
 * \file DialogFactory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "DialogFactory.h"
#include "FindAndReplace.h"
#include "GuiAbout.h"
#include "GuiBibitem.h"
#include "GuiBibtex.h"
#include "GuiBox.h"
#include "GuiBranch.h"
#include "GuiChanges.h"
#include "GuiCharacter.h"
#include "GuiCitation.h"
#include "GuiCompare.h"
#include "GuiCompareHistory.h"
#include "GuiCounter.h"
#include "GuiDelimiter.h"
#include "GuiDocument.h"
#include "GuiErrorList.h"
#include "GuiERT.h"
#include "GuiExternal.h"
#include "FloatPlacement.h"
#include "GuiGraphics.h"
#include "GuiHSpace.h"
#include "GuiHyperlink.h"
#include "GuiInclude.h"
#include "GuiIndex.h"
#include "GuiInfo.h"
#include "GuiLabel.h"
#include "GuiLine.h"
#include "GuiListings.h"
#include "GuiLog.h"
#include "GuiLyXFiles.h"
#include "GuiMathMatrix.h"
#include "GuiNomenclature.h"
#include "GuiNote.h"
#include "GuiParagraph.h"
#include "GuiPhantom.h"
#include "GuiPrefs.h"
#include "GuiPrintindex.h"
#include "GuiPrintNomencl.h"
#include "GuiProgressView.h"
#include "GuiRef.h"
#include "GuiSearch.h"
#include "GuiSendto.h"
#include "GuiShowFile.h"
#include "GuiSpellchecker.h"
#include "GuiSymbols.h"
#include "GuiTabular.h"
#include "GuiTabularCreate.h"
#include "GuiTexinfo.h"
#include "GuiThesaurus.h"
#include "GuiToc.h"
#include "GuiViewSource.h"
#include "GuiVSpace.h"
#include "GuiWrap.h"

#include "insets/Inset.h"
#include "InsetParamsDialog.h"
#include "InsetParamsWidget.h"

using namespace std;

namespace lyx {
namespace frontend {


Dialog * createDialog(GuiView & gv, string const & name)
{
	InsetParamsWidget * widget;
	switch (insetCode(name)) {
	case ERT_CODE:
		widget = new GuiERT;
		break;
	case FLOAT_CODE:
		widget = new FloatPlacement(true);
		break;
	case BIBITEM_CODE:
		widget = new GuiBibitem;
		break;
	case BRANCH_CODE:
		widget = new GuiBranch;
		break;
	case BOX_CODE:
		widget = new GuiBox;
		break;
	case HYPERLINK_CODE:
		widget = new GuiHyperlink;
		break;
	case COUNTER_CODE:
		widget = new GuiCounter(gv, nullptr);
		break;
	case INFO_CODE:
		widget = new GuiInfo;
		break;
	case LABEL_CODE:
		widget = new GuiLabel;
		break;
	case LINE_CODE:
		widget = new GuiLine;
		break;
	case MATH_SPACE_CODE:
		widget = new GuiHSpace(true);
		break;
	case NOMENCL_CODE:
		widget = new GuiNomenclature;
		break;
	case NOMENCL_PRINT_CODE:
		widget = new GuiPrintNomencl;
		break;
	case SPACE_CODE:
		widget = new GuiHSpace(false);
		break;
	case TABULAR_CODE:
		widget = new GuiTabular;
		break;
	case VSPACE_CODE:
		widget = new GuiVSpace;
		break;
	default:
		widget = nullptr;
	}
	if (widget)
		return new InsetParamsDialog(gv, widget);

	if (name == "aboutlyx")
		return new GuiAbout(gv);
	if (name == "bibtex")
		return new GuiBibtex(gv);
	if (name == "changes")
		return new GuiChanges(gv);
	if (name == "character")
		return new GuiCharacter(gv);
	if (name == "citation")
		return new GuiCitation(gv);
	if (name == "compare")
		return new GuiCompare(gv);
	if (name == "comparehistory")
		return new GuiCompareHistory(gv);
	if (name == "document")
		return new GuiDocument(gv);
	if (name == "errorlist")
		return new GuiErrorList(gv);
	if (name == "external")
		return new GuiExternal(gv);
	if (name == "file")
		return new GuiShowFile(gv);
	if (name == "findreplace")
		return new GuiSearch(gv);
	if (name == "findreplaceadv")
		return new FindAndReplace(gv);
	if (name == "graphics")
		return new GuiGraphics(gv);
	if (name == "include")
		return new GuiInclude(gv);
	if (name == "index")
		return new GuiIndex(gv);
	if (name == "index_print")
		return new GuiPrintindex(gv);
	if (name == "listings")
		return new GuiListings(gv);
	if (name == "log")
		return new GuiLog(gv);
	if (name == "lyxfiles")
		return new GuiLyXFiles(gv);
	if (name == "mathdelimiter")
		return new GuiDelimiter(gv);
	if (name == "mathmatrix")
		return new GuiMathMatrix(gv);
	if (name == "note")
		return new GuiNote(gv);
	if (name == "paragraph")
		return new GuiParagraph(gv);
	if (name == "phantom")
		return new GuiPhantom(gv);
	if (name == "prefs")
		return new GuiPreferences(gv);
	if (name == "ref")
		return new GuiRef(gv);
	if (name == "sendto")
		return new GuiSendTo(gv);
	if (name == "spellchecker")
		return new GuiSpellchecker(gv);
	if (name == "symbols")
		return new GuiSymbols(gv);
	if (name == "tabularcreate")
		return new GuiTabularCreate(gv);
	if (name == "texinfo")
		return new GuiTexInfo(gv);
	if (name == "thesaurus")
		return new GuiThesaurus(gv);
	if (name == "toc")
		return new GuiToc(gv);
	if (name == "view-source")
		return new GuiViewSource(gv);
	if (name == "wrap")
		return new GuiWrap(gv);
	if (name == "progress")
		return new GuiProgressView(gv);

	return nullptr;
}


} // namespace frontend
} // namespace lyx
