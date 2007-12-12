/**
 * \file factory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "factory.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "Lexer.h"
#include "LyX.h"
#include "TextClass.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetBibtex.h"
#include "insets/InsetCaption.h"
#include "insets/InsetCitation.h"
#include "insets/InsetFlex.h"
#include "insets/InsetEnvironment.h"
#include "insets/InsetERT.h"
#include "insets/InsetListings.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetFloatList.h"
#include "insets/InsetFoot.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetHFill.h"
#include "insets/InsetInclude.h"
#include "insets/InsetIndex.h"
#include "insets/InsetInfo.h"
#include "insets/InsetNomencl.h"
#include "insets/InsetLabel.h"
#include "insets/InsetLine.h"
#include "insets/InsetMarginal.h"
#include "insets/InsetNote.h"
#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetOptArg.h"
#include "insets/InsetNewpage.h"
#include "insets/InsetRef.h"
#include "insets/InsetSpace.h"
#include "insets/InsetTabular.h"
#include "insets/InsetTOC.h"
#include "insets/InsetHyperlink.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

#include "mathed/MathMacroTemplate.h"
#include "mathed/InsetMathHull.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"

#include <boost/assert.hpp>

#include <sstream>

using namespace std;

namespace lyx {

namespace Alert = frontend::Alert;


Inset * createInset(Buffer & buf, FuncRequest const & cmd)
{
	BufferParams const & params = buf.params();

	try {

		switch (cmd.action) {
		case LFUN_HFILL_INSERT:
			return new InsetHFill;

		case LFUN_LINE_INSERT:
			return new InsetLine;

		case LFUN_NEWPAGE_INSERT:
			return new InsetNewpage;

		case LFUN_PAGEBREAK_INSERT:
			return new InsetPagebreak;

		case LFUN_CLEARPAGE_INSERT:
			return new InsetClearPage;

		case LFUN_CLEARDOUBLEPAGE_INSERT:
			return new InsetClearDoublePage;

		case LFUN_FLEX_INSERT: {
			string s = cmd.getArg(0);
			TextClass const & tclass = params.getTextClass();
			InsetLayout const & il = tclass.insetlayout(from_utf8(s));
			return new InsetFlex(params, il);
		}

		case LFUN_NOTE_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Note";
			return new InsetNote(params, arg);
		}

		case LFUN_BOX_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Boxed";
			return new InsetBox(params, arg);
		}

		case LFUN_BRANCH_INSERT: {
			docstring arg = cmd.argument();
			if (arg.empty())
				arg = from_ascii("none");
			return new InsetBranch(params, InsetBranchParams(arg));
		}

		case LFUN_ERT_INSERT:
			return new InsetERT(params);

		case LFUN_LISTING_INSERT:
			return new InsetListings(params);

		case LFUN_FOOTNOTE_INSERT:
			return new InsetFoot(params);

		case LFUN_MARGINALNOTE_INSERT:
			return new InsetMarginal(params);

		case LFUN_OPTIONAL_INSERT:
			return new InsetOptArg(params);

		case LFUN_BIBITEM_INSERT:
			return new InsetBibitem(InsetCommandParams(BIBITEM_CODE));

		case LFUN_FLOAT_INSERT: {
			// check if the float type exists
			string const argument = to_utf8(cmd.argument());
			if (params.getTextClass().floats().typeExist(argument))
				return new InsetFloat(params, argument);
			lyxerr << "Non-existent float type: " << argument << endl;
		}

		case LFUN_FLOAT_WIDE_INSERT: {
			// check if the float type exists
			string const argument = to_utf8(cmd.argument());
			if (params.getTextClass().floats().typeExist(argument)) {
				auto_ptr<InsetFloat> p(new InsetFloat(params, argument));
				p->wide(true, params);
				return p.release();
			}
			lyxerr << "Non-existent float type: " << argument << endl;
			return 0;
		}

		case LFUN_WRAP_INSERT: {
			string const argument = to_utf8(cmd.argument());
			if (argument == "figure" || argument == "table")
				return new InsetWrap(params, argument);
			lyxerr << "Non-existent wrapfig type: " << argument << endl;
			return 0;
		}

		case LFUN_INDEX_INSERT:
			return new InsetIndex(params);

		case LFUN_NOMENCL_INSERT: {
			InsetCommandParams icp(NOMENCL_CODE);
			icp["symbol"] = cmd.argument();
			return new InsetNomencl(icp);
		}

		case LFUN_TABULAR_INSERT: {
			if (cmd.argument().empty())
				return 0;
			istringstream ss(to_utf8(cmd.argument()));
			int r = 0, c = 0;
			ss >> r >> c;
			if (r <= 0)
				r = 2;
			if (c <= 0)
				c = 2;
			return new InsetTabular(buf, r, c);
		}

		case LFUN_CAPTION_INSERT: {
			auto_ptr<InsetCaption> inset(new InsetCaption(params));
			inset->setAutoBreakRows(true);
			inset->setDrawFrame(true);
			inset->setFrameColor(Color_captionframe);
			return inset.release();
		}

		case LFUN_INDEX_PRINT:
			return new InsetPrintIndex(InsetCommandParams(INDEX_PRINT_CODE));

		case LFUN_NOMENCL_PRINT:
			return new InsetPrintNomencl(InsetCommandParams(NOMENCL_PRINT_CODE));

		case LFUN_TOC_INSERT:
			return new InsetTOC(InsetCommandParams(TOC_CODE));

		case LFUN_ENVIRONMENT_INSERT:
			return new InsetEnvironment(params, cmd.argument());

		case LFUN_INFO_INSERT:
			return new InsetInfo(params, to_utf8(cmd.argument()));
#if 0
		case LFUN_THEOREM_INSERT:
			return new InsetTheorem;
#endif

		case LFUN_INSET_INSERT: {
			string const name = cmd.getArg(0);
			InsetCode code = insetCode(name);
			switch (code) {
			case NO_CODE:
				lyxerr << "No such inset '" << name << "'.";
				return 0;
			
			case BIBITEM_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetBibitem(icp);
			}
			
			case BIBTEX_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetBibtex(icp);
			}
			
			case CITE_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetCitation(icp);
			}
			
			case ERT_CODE: {
				InsetCollapsable::CollapseStatus st;
				InsetERTMailer::string2params(to_utf8(cmd.argument()), st);
				return new InsetERT(params, st);
			}
				
			case LISTINGS_CODE: {
				InsetListingsParams par;
				InsetListingsMailer::string2params(to_utf8(cmd.argument()), par);
				return new InsetListings(params, par);
			}
			
			case EXTERNAL_CODE: {
				InsetExternalParams iep;
				InsetExternalMailer::string2params(to_utf8(cmd.argument()), buf, iep);
				auto_ptr<InsetExternal> inset(new InsetExternal);
				inset->setParams(iep, buf);
				return inset.release();
			}
			
			case GRAPHICS_CODE: {
				InsetGraphicsParams igp;
				InsetGraphicsMailer::string2params(to_utf8(cmd.argument()), buf, igp);
				auto_ptr<InsetGraphics> inset(new InsetGraphics);
				inset->setParams(igp);
				return inset.release();
			}
			
			case HYPERLINK_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetHyperlink(icp);
			}
			
			case INCLUDE_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetInclude(icp);
			}
			
			case INDEX_CODE:
				return new InsetIndex(params);
			
			case NOMENCL_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, lyx::to_utf8(cmd.argument()), icp);
				return new InsetNomencl(icp);
			}
			
			case LABEL_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetLabel(icp);
			}
			
			case REF_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetRef(icp, buf);
			}
			
			case TOC_CODE: {
				InsetCommandParams icp(code);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), icp);
				return new InsetTOC(icp);
			}
			
			case VSPACE_CODE: {
				VSpace vspace;
				InsetVSpaceMailer::string2params(to_utf8(cmd.argument()), vspace);
				return new InsetVSpace(vspace);
			}
			
			default:
				lyxerr << "Inset '" << name << "' not permitted with LFUN_INSET_INSERT."
						<< endl;
				return 0;
			
			}
			} //end LFUN_INSET_INSERT

		case LFUN_SPACE_INSERT: {
			string const name = to_utf8(cmd.argument());
			if (name == "normal")
				return new InsetSpace(InsetSpace::NORMAL);
			if (name == "protected")
				return new InsetSpace(InsetSpace::PROTECTED);
			if (name == "thin")
				return new InsetSpace(InsetSpace::THIN);
			if (name == "quad")
				return new InsetSpace(InsetSpace::QUAD);
			if (name == "qquad")
				return new InsetSpace(InsetSpace::QQUAD);
			if (name == "enspace")
				return new InsetSpace(InsetSpace::ENSPACE);
			if (name == "enskip")
				return new InsetSpace(InsetSpace::ENSKIP);
			if (name == "negthinspace")
				return new InsetSpace(InsetSpace::NEGTHIN);
			if (name.empty())
				lyxerr << "LyX function 'space' needs an argument." << endl;
			else
				lyxerr << "Wrong argument for LyX function 'space'." << endl;
		}
		break;

		default:
			break;
		}

	} catch (support::ExceptionMessage const & message) {
		if (message.type_ == support::ErrorException) {
			Alert::error(message.title_, message.details_);
			LyX::cref().emergencyCleanup();
			abort();
		} else if (message.type_ == support::WarningException) {
			Alert::warning(message.title_, message.details_);
			return 0;
		}
	}


	return 0;
}


Inset * readInset(Lexer & lex, Buffer const & buf)
{
	// consistency check
	if (lex.getString() != "\\begin_inset") {
		lyxerr << "Buffer::readInset: Consistency check failed."
		       << endl;
	}

	auto_ptr<Inset> inset;

	TextClass const & tclass = buf.params().getTextClass();

	lex.next();
	string tmptok = lex.getString();

	// test the different insets
	
	//FIXME It would be better if we did not have this branch and could
	//just do one massive switch for all insets. But at present, it's easier 
	//to do it this way, and we can't do the massive switch until the conversion 
	//mentioned below. 
	//Note that if we do want to do a single switch, we need to remove
	//this "CommandInset" line---or replace it with a single "InsetType" line
	//that would be used in all insets.
	if (tmptok == "CommandInset") {
		lex.next();
		string const insetType = lex.getString();
		lex.pushToken(insetType);
		
		InsetCode const code = insetCode(insetType);
		
		//FIXME If we do the one massive switch, we cannot do this here, since
		//we do not know in advance that we're dealing with a command inset.
		//Worst case, we could put it in each case below. Better, we could
		//pass the lexer to the constructor and let the params be built there.
		InsetCommandParams inscmd(code);
		inscmd.read(lex);

		switch (code) {
			case BIBITEM_CODE:
				inset.reset(new InsetBibitem(inscmd));
				break;
			case BIBTEX_CODE:
				inset.reset(new InsetBibtex(inscmd));
				break;
			case CITE_CODE: 
				inset.reset(new InsetCitation(inscmd));
				break;
			case HYPERLINK_CODE:
				inset.reset(new InsetHyperlink(inscmd));
				break;
			case INCLUDE_CODE:
				inset.reset(new InsetInclude(inscmd));
				break;
			case INDEX_PRINT_CODE:
				inset.reset(new InsetPrintIndex(inscmd));
				break;
			case LABEL_CODE:
				inset.reset(new InsetLabel(inscmd));
				break;
			case NOMENCL_CODE:
				inset.reset(new InsetNomencl(inscmd));
				break;
			case NOMENCL_PRINT_CODE:
				inset.reset(new InsetPrintNomencl(inscmd));
				break;
			case REF_CODE:
				if (!inscmd["name"].empty() || !inscmd["reference"].empty())
					inset.reset(new InsetRef(inscmd, buf));
				break;
			case TOC_CODE:
				inset.reset(new InsetTOC(inscmd));
				break;
			case NO_CODE:
			default:
				lyxerr << "unknown CommandInset '" << insetType
							<< "'" << endl;
				while (lex.isOK() && lex.getString() != "\\end_inset")
					lex.next();
				return 0;
		}
	} else { 
		// FIXME This branch should be made to use inset codes as the preceding 
		// branch does. Unfortunately, that will take some doing. It requires
		// converting the representation of the insets in LyX files so that they
		// use the inset names listed in Inset.cpp. Then, as above, the inset names
		// can be translated to inset codes using insetCode(). And the insets'
		// write() routines should use insetName() rather than hardcoding it.
		if (tmptok == "Quotes") {
			inset.reset(new InsetQuotes);
		} else if (tmptok == "External") {
			inset.reset(new InsetExternal);
		} else if (tmptok == "FormulaMacro") {
			inset.reset(new MathMacroTemplate);
		} else if (tmptok == "Formula") {
			inset.reset(new InsetMathHull);
		} else if (tmptok == "Graphics") {
			inset.reset(new InsetGraphics);
		} else if (tmptok == "Note") {
			inset.reset(new InsetNote(buf.params(), tmptok));
		} else if (tmptok == "Box") {
			inset.reset(new InsetBox(buf.params(), tmptok));
		} else if (tmptok == "Flex") {
			lex.next();
			string s = lex.getString();
			InsetLayout const & il = tclass.insetlayout(from_utf8(s));
			inset.reset(new InsetFlex(buf.params(), il));
		} else if (tmptok == "Branch") {
			inset.reset(new InsetBranch(buf.params(),
						    InsetBranchParams()));
		} else if (tmptok == "Environment") {
			lex.next();
			inset.reset(new InsetEnvironment(buf.params(), lex.getDocString()));
		} else if (tmptok == "ERT") {
			inset.reset(new InsetERT(buf.params()));
		} else if (tmptok == "listings") {
			inset.reset(new InsetListings(buf.params()));
		} else if (tmptok == "InsetSpace") {
			inset.reset(new InsetSpace);
		} else if (tmptok == "Tabular") {
			inset.reset(new InsetTabular(buf));
		} else if (tmptok == "Text") {
			inset.reset(new InsetText(buf.params()));
		} else if (tmptok == "VSpace") {
			inset.reset(new InsetVSpace);
		} else if (tmptok == "Foot") {
			inset.reset(new InsetFoot(buf.params()));
		} else if (tmptok == "Marginal") {
			inset.reset(new InsetMarginal(buf.params()));
		} else if (tmptok == "OptArg") {
			inset.reset(new InsetOptArg(buf.params()));
		} else if (tmptok == "Float") {
			lex.next();
			string tmptok = lex.getString();
			inset.reset(new InsetFloat(buf.params(), tmptok));
		} else if (tmptok == "Wrap") {
			lex.next();
			string tmptok = lex.getString();
			inset.reset(new InsetWrap(buf.params(), tmptok));
#if 0
		} else if (tmptok == "Theorem") {
			inset.reset(new InsetList);
#endif
		} else if (tmptok == "Caption") {
			inset.reset(new InsetCaption(buf.params()));
		} else if (tmptok == "Index") {
			inset.reset(new InsetIndex(buf.params()));
		} else if (tmptok == "FloatList") {
			inset.reset(new InsetFloatList);
		} else if (tmptok == "Info") {
			inset.reset(new InsetInfo(buf.params()));
		} else {
			lyxerr << "unknown Inset type '" << tmptok
			       << "'" << endl;
			while (lex.isOK() && lex.getString() != "\\end_inset")
				lex.next();
			return 0;
		}

		inset->read(buf, lex);
	}

	return inset.release();
}


} // namespace lyx
