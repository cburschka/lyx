
#include "funcrequest.h"
#include "bufferparams.h"
#include "buffer.h"
#include "FloatList.h"
#include "debug.h"
#include "BufferView.h"
#include "lyxtext.h"

#include "insets/insetcaption.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetfoot.h"
#include "insets/insetindex.h"
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
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include <cstdio>


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

		case LFUN_INSET_EXTERNAL:
			return new InsetExternal;

		case LFUN_INSET_FOOTNOTE:
			return new InsetFoot(params);

		case LFUN_INSET_MARGINAL:
			return new InsetMarginal(params);

		case LFUN_INSET_OPTARG:
			return new InsetOptArg(params);

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

		case LFUN_INDEX_INSERT: {
			string entry = cmd.argument;
			if (entry.empty())
				entry = bv->getLyXText()->getStringToIndex(bv);
			if (!entry.empty())
				return new InsetIndex(InsetCommandParams("index", entry));
			bv->owner()->getDialogs().createIndex();
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

		case LFUN_INSERT_URL:
		{
			InsetCommandParams p;
			p.setFromString(cmd.argument);
			return new InsetUrl(p);
		}

	#if 0
		case LFUN_INSET_LIST:
			return new InsetList;

		case LFUN_INSET_THEOREM:
			return new InsetTheorem;
	#endif

		default:
			break;
	}
	return 0;
}


