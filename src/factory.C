
#include "funcrequest.h"
#include "bufferparams.h"
#include "buffer.h"
#include "FloatList.h"
#include "debug.h"
#include "BufferView.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetnote.h"
#include "insets/insetoptarg.h"
#include "insets/insetparent.h"
#include "insets/insetref.h"
#include "insets/insettext.h"


Inset * createInset(FuncRequest const & cmd)
{
	BufferParams const & params = cmd.view()->buffer()->params;

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


