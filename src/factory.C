/**
 * \file factory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "funcrequest.h"
#include "bufferparams.h"
#include "buffer.h"
#include "FloatList.h"
#include "debug.h"
#include "BufferView.h"
#include "lyxtext.h"

#include "insets/insetbibitem.h"
#include "insets/insetbibtex.h"
#include "insets/insetcaption.h"
#include "insets/insetcite.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetfoot.h"
#include "insets/insetinclude.h"
#include "insets/insetindex.h"
#include "insets/insetlabel.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetnote.h"
#include "insets/insetoptarg.h"
#include "insets/insetparent.h"
#include "insets/insetref.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"
#include "insets/insettoc.h"
#include "insets/inseturl.h"
#include "insets/insetwrap.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include <cstdio>

using std::endl;

Inset * createInset(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	BufferParams const & params = bv->buffer()->params;

	switch (cmd.action) {
	case LFUN_INSET_MINIPAGE:
		return new InsetMinipage(params);

	case LFUN_INSERT_NOTE:
		return new InsetNote(params);

	case LFUN_INSET_ERT:
		return new InsetERT(params);

	case LFUN_INSET_FOOTNOTE:
		return new InsetFoot(params);

	case LFUN_INSET_MARGINAL:
		return new InsetMarginal(params);

	case LFUN_INSET_OPTARG:
		return new InsetOptArg(params);

	case LFUN_INSERT_BIBITEM:
		return new InsetBibitem(InsetCommandParams("bibitem"));

	case LFUN_INSET_FLOAT:
		// check if the float type exists
		if (params.getLyXTextClass().floats().typeExist(cmd.argument))
			return new InsetFloat(params, cmd.argument);
		lyxerr << "Non-existent float type: " << cmd.argument << endl;
		return 0;

	case LFUN_INSET_WIDE_FLOAT:
		// check if the float type exists
		if (params.getLyXTextClass().floats().typeExist(cmd.argument)) {
			InsetFloat * p = new InsetFloat(params, cmd.argument);
			p->wide(true, params);
		}
		lyxerr << "Non-existent float type: " << cmd.argument << endl;
		return 0;

	case LFUN_INSET_WRAP:
		if (cmd.argument == "figure")
			return new InsetWrap(params, cmd.argument);
		lyxerr << "Non-existent floatflt type: " << cmd.argument << endl;
		return 0;

	case LFUN_INDEX_INSERT: {
		// Try and generate a valid index entry.
		InsetCommandParams icp("index");
		string const contents = cmd.argument.empty() ?
			bv->getLyXText()->getStringToIndex(bv) :
			cmd.argument;
		icp.setContents(contents);

		string data = InsetCommandMailer::params2string("index", icp);
		LyXView * lv = bv->owner();

		if (icp.getContents().empty()) {
			lv->getDialogs().show("index", data, 0);
		} else {
			FuncRequest fr(bv, LFUN_INSET_APPLY, data);
			lv->dispatch(fr);
		}
		return 0;
	}

	case LFUN_TABULAR_INSERT:
		if (!cmd.argument.empty()) {
			int r = 2;
			int c = 2;
			::sscanf(cmd.argument.c_str(),"%d%d", &r, &c);
			return new InsetTabular(*bv->buffer(), r, c);
		}
		bv->owner()->getDialogs().showTabularCreate();
		return 0;

	case LFUN_INSET_CAPTION:
		if (bv->theLockingInset()) {
			lyxerr << "Locking inset code: "
			       << static_cast<int>(bv->theLockingInset()->lyxCode());
			InsetCaption * inset = new InsetCaption(params);
			inset->setOwner(bv->theLockingInset());
			inset->setAutoBreakRows(true);
			inset->setDrawFrame(0, InsetText::LOCKED);
			inset->setFrameColor(0, LColor::captionframe);
			return inset;
		}
		return 0;

	case LFUN_INDEX_PRINT:
		return new InsetPrintIndex(InsetCommandParams("printindex"));

	case LFUN_TOC_INSERT:
		return new InsetTOC(InsetCommandParams("tableofcontents"));

	case LFUN_PARENTINSERT:
		return new InsetParent(
			InsetCommandParams("lyxparent", cmd.argument), *bv->buffer());

#if 0
	case LFUN_INSET_LIST:
		return new InsetList;

	case LFUN_INSET_THEOREM:
		return new InsetTheorem;
#endif

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);

		if (name == "bibitem") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetBibitem(icp);

		} else if (name == "bibtex") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetBibtex(icp);

		} else if (name == "citation") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			InsetCitation * inset = new InsetCitation(icp);
			inset->setLoadingBuffer(bv->buffer(), false);
			return inset;

		} else if (name == "ert") {
			InsetERT * inset = new InsetERT(params);
			InsetERT::ERTStatus s;
			InsetERTMailer::string2params(cmd.argument, s);
			inset->status(bv, s);
			return inset;

		} else if (name == "external") {
			InsetExternal::Params iep;			
			InsetExternalMailer::string2params(cmd.argument, iep);
			InsetExternal * inset = new InsetExternal;
			inset->setFromParams(iep);
			return inset;

		} else if (name == "include") {
			InsetInclude::Params iip;
			InsetIncludeMailer::string2params(cmd.argument, iip);
			return new InsetInclude(iip);

		} else if (name == "index") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetIndex(icp);

		} else if (name == "label") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetLabel(icp);

		} else if (name == "ref") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetRef(icp, *bv->buffer());

		} else if (name == "toc") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetTOC(icp);

		} else if (name == "url") {
			InsetCommandParams icp;
			InsetCommandMailer::string2params(cmd.argument, icp);
			return new InsetUrl(icp);
		}
	}
	break;

	default:
		break;
	}

	return 0;
}
