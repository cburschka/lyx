/**
 * \file xforms/Dialogs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Dialogs.h"
#include "Dialog.h"

#include "Tooltips.h"

#include "ControlAboutlyx.h"
#include "ControlBibtex.h"
#include "ControlBranch.h"
#include "ControlChanges.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlCommand.h"
#include "ControlErrorList.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlGraphics.h"
#include "ControlInclude.h"
#include "ControlLog.h"
#include "ControlMath.h"
#include "ControlNote.h"
#include "ControlParagraph.h"
#include "ControlRef.h"
#include "ControlShowFile.h"
#include "ControlTabular.h"
#include "ControlTabularCreate.h"
#include "ControlTexinfo.h"
#include "ControlToc.h"
#include "ControlWrap.h"

#include "GAboutlyx.h"
#include "GText.h"
#include "GMathDelim.h"
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormBranch.h"
#include "FormChanges.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormErrorList.h"
#include "FormERT.h"
#include "FormExternal.h"
#include "FormFloat.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormLog.h"
#include "GMathPanel.h"
#include "FormMathsBitmap.h"
#include "FormMathsMatrix.h"
#include "FormMathsSpace.h"
#include "FormMathsStyle.h"
#include "FormNote.h"
#include "FormParagraph.h"
#include "FormRef.h"
#include "FormTabular.h"
#include "FormTexinfo.h"
#include "FormShowFile.h"
#include "GTableCreate.h"
#include "FormToc.h"
#include "GUrl.h"
#include "FormWrap.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#endif

#include "xformsBC.h"
#include "ButtonController.h"

#include "arrows.xbm"
#include "bop.xbm"
#include "brel.xbm"
#include "deco.xbm"
#include "dots.xbm"
#include "greek.xbm"
#include "misc.xbm"
#include "varsz.xbm"

#include "ams_misc.xbm"
#include "ams_arrows.xbm"
#include "ams_rel.xbm"
#include "ams_nrel.xbm"
#include "ams_ops.xbm"

#include <vector>

using std::string;


namespace {

FormMathsBitmap * createFormBitmap(Dialog & parent, string const & title,
				   char const * const * data, int size)
{
	char const * const * const end = data + size;
	return new FormMathsBitmap(parent, title, std::vector<string>(data, end));
}


char const * const dialognames[] = { "aboutlyx", "bibitem", "bibtex", "branch", "changes",
"character", "citation", "error", "errorlist" , "ert", "external", "file",
"float", "graphics", "include", "index", "label", "latexlog", "mathpanel",
"mathaccents", "matharrows", "mathoperators", "mathrelations", "mathgreek",
"mathmisc", "mathdots", "mathbigoperators", "mathamsmisc",
"mathamsarrows", "mathamsrelations", "mathamsnegatedrelations", "mathamsoperators",
"mathdelimiter", "mathmatrix", "mathspace", "mathstyle",
"box", "note", "paragraph", "ref", "tabular", "tabularcreate", "texinfo",

#ifdef HAVE_LIBAIKSAURUS
"thesaurus",
#endif

"toc", "url", "wrap" };

char const * const * const end_dialognames =
	dialognames + (sizeof(dialognames) / sizeof(char *));

struct cmpCStr {
	cmpCStr(char const * name) : name_(name) {}
	bool operator()(char const * other) {
		return strcmp(other, name_) == 0;
	}
private:
	char const * name_;
};

} // namespace anon


bool Dialogs::isValidName(string const & name) const
{
	return std::find_if(dialognames, end_dialognames,
			    cmpCStr(name.c_str())) != end_dialognames;
}


Dialog * Dialogs::build(string const & name)
{
	if (!isValidName(name))
		return 0;

	Dialog * dialog = new Dialog(lyxview_, name);
	dialog->bc().view(new xformsBC(dialog->bc()));

	if (name == "aboutlyx") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlAboutlyx(*dialog));
		dialog->setView(new GAboutlyx(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "bibitem") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormBibitem(*dialog));
		dialog->bc().bp(new OkCancelReadOnlyPolicy);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new FormBibtex(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "character") {
		dialog->setController(new ControlCharacter(*dialog));
		dialog->setView(new FormCharacter(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "changes") {
		dialog->setController(new ControlChanges(*dialog));
		dialog->setView(new FormChanges(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new FormCitation(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "errorlist") {
		dialog->setController(new ControlErrorList(*dialog));
		dialog->setView(new FormErrorList(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new FormERT(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new FormExternal(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "file") {
		dialog->setController(new ControlShowFile(*dialog));
		dialog->setView(new FormShowFile(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "float") {
		dialog->setController(new ControlFloat(*dialog));
		dialog->setView(new FormFloat(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "graphics") {
		dialog->setController(new ControlGraphics(*dialog));
		dialog->setView(new FormGraphics(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "include") {
		dialog->setController(new ControlInclude(*dialog));
		dialog->setView(new FormInclude(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "index") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new GText(*dialog,
					  _("Index"), _("Keyword:|#K")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "label") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new GText(*dialog,
					  _("Label"), _("Label:|#L")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "latexlog") {
		dialog->setController(new ControlLog(*dialog));
		dialog->setView(new FormLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);

	} else if (name == "mathpanel") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new GMathPanel(*dialog));
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathaccents") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Maths Decorations & Accents"),
					 latex_deco, nr_latex_deco);
		bitmap->addBitmap(
			BitmapStore(12, 3, 4, deco1_width, deco1_height, deco1_bits, true));
		bitmap->addBitmap(
			BitmapStore(10, 4, 3, deco2_width, deco2_height, deco2_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "matharrows") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Arrows"), latex_arrow, nr_latex_arrow);
		bitmap->addBitmap(
			BitmapStore(20, 5, 4, arrow_width,  arrow_height,  arrow_bits, true));
		bitmap->addBitmap(
			BitmapStore(7,  2, 4, larrow_width, larrow_height, larrow_bits, false));
		bitmap->addBitmap(
			BitmapStore(4,  2, 2, darrow_width,  darrow_height, darrow_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathoperators") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Binary Ops"),
					 latex_bop, nr_latex_bop);
		bitmap->addBitmap(
			BitmapStore(31, 4, 8, bop_width, bop_height, bop_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathrelations") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Binary Relations"),
					 latex_brel, nr_latex_brel);
		bitmap->addBitmap(
			BitmapStore(35, 4, 9, brel_width, brel_height, brel_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathgreek") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Greek"),
					 latex_greek, nr_latex_greek);
		bitmap->addBitmap(
			BitmapStore(11, 6, 2, Greek_width, Greek_height, Greek_bits, true));
		bitmap->addBitmap(
			BitmapStore(28, 7, 4, greek_width, greek_height, greek_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathmisc") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Misc"),
					 latex_misc, nr_latex_misc);
		bitmap->addBitmap(
			BitmapStore(29, 5, 6, misc_width, misc_height, misc_bits, true));
		bitmap->addBitmap(
			BitmapStore(5, 5, 1, misc4_width, misc4_height, misc4_bits, true));
		bitmap->addBitmap(
			BitmapStore(6, 3, 2, misc2_width, misc2_height, misc2_bits, false));
		bitmap->addBitmap(
			BitmapStore(4, 2, 2, misc3_width, misc3_height, misc3_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathdots") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Dots"),
					 latex_dots, nr_latex_dots);
		bitmap->addBitmap(
			BitmapStore(4, 4, 1, dots_width, dots_height, dots_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathbigoperators") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("Big Operators"),
					 latex_varsz, nr_latex_varsz);
		bitmap->addBitmap(
			BitmapStore(14, 3, 5, varsz_width, varsz_height, varsz_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathamsmisc") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("AMS Misc"),
					 latex_ams_misc, nr_latex_ams_misc);
		bitmap->addBitmap(
			BitmapStore(9, 5, 2, ams1_width, ams1_height, ams1_bits, true));
		bitmap->addBitmap(
			BitmapStore(26, 3, 9, ams7_width, ams7_height, ams7_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathamsarrows") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("AMS Arrows"),
					 latex_ams_arrows, nr_latex_ams_arrows);
		bitmap->addBitmap(
			BitmapStore(32, 3, 11, ams2_width, ams2_height, ams2_bits, true));
		bitmap->addBitmap(
			BitmapStore(6, 3, 2, ams3_width, ams3_height, ams3_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathamsrelations") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("AMS Relations"),
					 latex_ams_rel, nr_latex_ams_rel);
		bitmap->addBitmap(
			BitmapStore(66, 6, 11, ams_rel_width, ams_rel_height, ams_rel_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathamsnegatedrelations") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("AMS Negated Rel"),
					 latex_ams_nrel, nr_latex_ams_nrel);
		bitmap->addBitmap(
			BitmapStore(51, 6, 9, ams_nrel_width, ams_nrel_height, ams_nrel_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathamsoperators") {
		FormMathsBitmap * bitmap =
			createFormBitmap(*dialog, _("AMS Operators"),
					 latex_ams_ops, nr_latex_ams_ops);
		bitmap->addBitmap(
			BitmapStore(23, 3, 8, ams_ops_width, ams_ops_height, ams_ops_bits, true));

		dialog->setController(new ControlMath(*dialog));
		dialog->setView(bitmap);
		dialog->bc().bp(new IgnorantPolicy);

	} else if (name == "mathdelimiter") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new GMathDelim(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "mathmatrix") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new FormMathsMatrix(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "mathspace") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new FormMathsSpace(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "mathstyle") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new FormMathsStyle(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "note") {
		dialog->setController(new ControlNote(*dialog));
		dialog->setView(new FormNote(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "branch") {
		dialog->setController(new ControlBranch(*dialog));
		dialog->setView(new FormBranch(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "paragraph") {
		dialog->setController(new ControlParagraph(*dialog));
		dialog->setView(new FormParagraph(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new FormRef(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabular") {
		dialog->setController(new ControlTabular(*dialog));
		dialog->setView(new FormTabular(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabularcreate") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlTabularCreate(*dialog));
		dialog->setView(new GTableCreate(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "texinfo") {
		dialog->setController(new ControlTexinfo(*dialog));
		dialog->setView(new FormTexinfo(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
#ifdef HAVE_LIBAIKSAURUS
	} else if (name == "thesaurus") {
		dialog->setController(new ControlThesaurus(*dialog));
		dialog->setView(new FormThesaurus(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
#endif
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new FormToc(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "url") {
		dialog->bc().view(new GBC(dialog->bc()));
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new GUrl(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "wrap") {
		dialog->setController(new ControlWrap(*dialog));
		dialog->setView(new FormWrap(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	}

	return dialog;
}


void Dialogs::toggleTooltips()
{
	Tooltips::toggleEnabled();
}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}
