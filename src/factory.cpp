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
#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetCaption.h"
#include "insets/InsetCitation.h"
#include "insets/InsetFlex.h"
#include "insets/InsetERT.h"
#include "insets/InsetListings.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetFloatList.h"
#include "insets/InsetFoot.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetHyperlink.h"
#include "insets/InsetInclude.h"
#include "insets/InsetIndex.h"
#include "insets/InsetInfo.h"
#include "insets/InsetIPA.h"
#include "insets/InsetIPAMacro.h"
#include "insets/InsetLabel.h"
#include "insets/InsetLine.h"
#include "insets/InsetMarginal.h"
#include "insets/InsetNewline.h"
#include "insets/InsetNewpage.h"
#include "insets/InsetNomencl.h"
#include "insets/InsetNote.h"
#include "insets/InsetArgument.h"
#include "insets/InsetPhantom.h"
#include "insets/InsetPreview.h"
#include "insets/InsetRef.h"
#include "insets/InsetScript.h"
#include "insets/InsetSeparator.h"
#include "insets/InsetSpace.h"
#include "insets/InsetTabular.h"
#include "insets/InsetTOC.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

#include "mathed/MathMacroTemplate.h"
#include "mathed/InsetMathHull.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/unique_ptr.h"

#include <sstream>


using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


Inset * createInsetHelper(Buffer * buf, FuncRequest const & cmd)
{
	try {

		switch (cmd.action()) {

		case LFUN_NEWPAGE_INSERT: {
			string const name = cmd.getArg(0);
			InsetNewpageParams inp;
			if (name.empty() || name == "newpage")
				inp.kind = InsetNewpageParams::NEWPAGE;
			else if (name == "pagebreak")
				inp.kind = InsetNewpageParams::PAGEBREAK;
			else if (name == "clearpage")
				inp.kind = InsetNewpageParams::CLEARPAGE;
			else if (name == "cleardoublepage")
				inp.kind = InsetNewpageParams::CLEARDOUBLEPAGE;
			return new InsetNewpage(inp);
		}

		case LFUN_SEPARATOR_INSERT: {
			string const name = cmd.getArg(0);
			InsetSeparatorParams inp;
			if (name.empty() || name == "plain")
				inp.kind = InsetSeparatorParams::PLAIN;
			else if (name == "parbreak")
				inp.kind = InsetSeparatorParams::PARBREAK;
			else if (name == "latexpar")
				inp.kind = InsetSeparatorParams::LATEXPAR;
			else {
				lyxerr << "Wrong argument for LyX function 'separator-insert'." << endl;
				break;
			}
			return new InsetSeparator(inp);
		}

		case LFUN_FLEX_INSERT: {
			string s = cmd.getArg(0);
			return new InsetFlex(buf, s);
		}

		case LFUN_NOTE_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Note";
			return new InsetNote(buf, arg);
		}

		case LFUN_BOX_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Boxed";
			return new InsetBox(buf, arg);
		}

		case LFUN_BRANCH_INSERT: {
			docstring arg = cmd.argument();
			if (arg.empty())
				arg = from_ascii("none");
			return new InsetBranch(buf, InsetBranchParams(arg));
		}

		case LFUN_PHANTOM_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Phantom";
			return new InsetPhantom(buf, arg);
		}

		case LFUN_IPAMACRO_INSERT: {
			string const arg1 = cmd.getArg(0);
			string const arg2 = cmd.getArg(1);
			if (arg1 != "deco") {
				LYXERR0("LFUN_IPAMACRO_INSERT: wrong argument");
				return 0;
			}
			return new InsetIPADeco(buf, arg2);
		}

		case LFUN_ERT_INSERT:
			return new InsetERT(buf);

		case LFUN_LISTING_INSERT:
			return new InsetListings(buf);

		case LFUN_FOOTNOTE_INSERT:
			return new InsetFoot(buf);

		case LFUN_MARGINALNOTE_INSERT:
			return new InsetMarginal(buf);

		case LFUN_ARGUMENT_INSERT: {
			string arg = cmd.getArg(0);
			if (arg.empty()) {
				LYXERR0("argument-insert needs an argument!");
				return 0;
			}
			return new InsetArgument(buf, arg);
		}

		case LFUN_FLOAT_INSERT: {
			string argument = to_utf8(cmd.argument());
			if (!argument.empty()) {
				if (!contains(argument, "sideways")) {
					if (!contains(argument, "wide"))
						argument += "\nwide false";
					argument += "\nsideways false";
				}
			}
			return new InsetFloat(buf, argument);
		}

		case LFUN_FLOAT_WIDE_INSERT: {
			string argument = to_utf8(cmd.argument());
			if (!argument.empty()) {
				if (!contains(argument, "sideways")) {
					if (!contains(argument, "wide"))
						argument += "\nwide true";
					argument += "\nsideways false";
				}
			}
			InsetFloat * fl = new InsetFloat(buf, argument);
			fl->setWide(true);
			return fl;
		}

		case LFUN_WRAP_INSERT: {
			string const argument = to_utf8(cmd.argument());
			if (argument == "figure" || argument == "table")
				return new InsetWrap(buf, argument);
			lyxerr << "Non-existent wrapfig type: " << argument << endl;
			return 0;
		}

		case LFUN_INDEX_INSERT: {
			docstring arg = cmd.argument();
			return new InsetIndex(buf, InsetIndexParams(arg));
		}

		case LFUN_IPA_INSERT:
			return new InsetIPA(buf);

		case LFUN_NOMENCL_INSERT: {
			InsetCommandParams icp(NOMENCL_CODE);
			icp["symbol"] = cmd.argument();
			return new InsetNomencl(buf, icp);
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
			string arg = cmd.getArg(0);
			if (arg.empty())
				arg = "Standard";
			return new InsetCaption(buf, arg);
		}

		case LFUN_INDEX_PRINT:  {
			InsetCommandParams icp(INDEX_PRINT_CODE);
			icp["type"] = cmd.argument();
			return new InsetPrintIndex(buf, icp);
		}

		case LFUN_NOMENCL_PRINT: {
			InsetCommandParams icp(NOMENCL_PRINT_CODE);
			icp["set_width"] = from_ascii("auto");
			return new InsetPrintNomencl(buf, icp);
		}

		case LFUN_INFO_INSERT: {
			InsetInfo * inset = new InsetInfo(buf, to_utf8(cmd.argument()));
			inset->updateInfo();
			return inset;
		}

		case LFUN_PREVIEW_INSERT:
			return new InsetPreview(buf);

		case LFUN_SCRIPT_INSERT: {
			InsetScriptParams isp;
			InsetScript::string2params("script script " + to_utf8(cmd.argument()), isp);
			return new InsetScript(buf, isp);
		}

		case LFUN_INSET_INSERT: {
			string const name = cmd.getArg(0);
			InsetCode code = insetCode(name);
			switch (code) {
			case NO_CODE:
				lyxerr << "No such inset '" << name << "'.";
				return 0;
			
			case BIBITEM_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetBibitem(buf, icp);
			}
			
			case BIBTEX_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetBibtex(buf, icp);
			}
			
			case CITE_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetCitation(buf, icp);
			}
			
			case ERT_CODE: {
				return new InsetERT(buf,
					InsetERT::string2params(to_utf8(cmd.argument())));
			}
			
			case EXTERNAL_CODE: {
				InsetExternalParams iep;
				InsetExternal::string2params(to_utf8(cmd.argument()), *buf, iep);
				auto inset = make_unique<InsetExternal>(buf);
				inset->setBuffer(*buf);
				inset->setParams(iep);
				return inset.release();
			}
			
			case GRAPHICS_CODE: {
				InsetGraphicsParams igp;
				InsetGraphics::string2params(to_utf8(cmd.argument()), *buf, igp);
				auto inset = make_unique<InsetGraphics>(buf);
				inset->setParams(igp);
				return inset.release();
			}
			
			case HYPERLINK_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetHyperlink(buf, icp);
			}
			
			case INCLUDE_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetInclude(buf, icp);
			}
			
			case INDEX_CODE: {
				docstring arg = cmd.argument();
				return new InsetIndex(buf, InsetIndexParams(arg));
			}
			
			case INDEX_PRINT_CODE:  {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetPrintIndex(buf, icp);
			}
			
			case LABEL_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetLabel(buf, icp);
			}
			
			case LINE_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetLine(buf, icp);
			}
				
			case LISTINGS_CODE: {
				InsetListingsParams par;
				InsetListings::string2params(to_utf8(cmd.argument()), par);
				return new InsetListings(buf, par);
			}
			
			case NOMENCL_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetNomencl(buf, icp);
			}
			
			case REF_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetRef(buf, icp);
			}

			case SCRIPT_CODE: {
				InsetScriptParams isp;
				InsetScript::string2params(to_utf8(cmd.argument()), isp);
				return new InsetScript(buf, isp);
			}

			case SPACE_CODE: {
				InsetSpaceParams isp;
				InsetSpace::string2params(to_utf8(cmd.argument()), isp);
				return new InsetSpace(isp);
			}
			
			case TOC_CODE: {
				InsetCommandParams icp(code);
				InsetCommand::string2params(to_utf8(cmd.argument()), icp);
				return new InsetTOC(buf, icp);
			}
			
			case VSPACE_CODE: {
				VSpace vspace;
				InsetVSpace::string2params(to_utf8(cmd.argument()), vspace);
				return new InsetVSpace(vspace);
			}

			case PREVIEW_CODE:
				return new InsetPreview(buf);
			
			default:
				lyxerr << "Inset '" << name << "' not permitted with LFUN_INSET_INSERT."
						<< endl;
				return 0;
			
			}
		} //end LFUN_INSET_INSERT

		case LFUN_SPACE_INSERT: {
			string const name = cmd.getArg(0);
			string const len = cmd.getArg(1);
			if (name.empty()) {
				lyxerr << "LyX function 'space-insert' needs an argument." << endl;
				break;
			}
			InsetSpaceParams isp;
			// The tests for isp.math might be disabled after a file format change
			if (name == "normal")
				isp.kind = InsetSpaceParams::NORMAL;
			else if (name == "protected")
				isp.kind = InsetSpaceParams::PROTECTED;
			else if (name == "visible")
				isp.kind = InsetSpaceParams::VISIBLE;
			else if (name == "thin")
				isp.kind = InsetSpaceParams::THIN;
			else if (isp.math && name == "med")
				isp.kind = InsetSpaceParams::MEDIUM;
			else if (isp.math && name == "thick")
				isp.kind = InsetSpaceParams::THICK;
			else if (name == "quad")
				isp.kind = InsetSpaceParams::QUAD;
			else if (name == "qquad")
				isp.kind = InsetSpaceParams::QQUAD;
			else if (name == "enspace")
				isp.kind = InsetSpaceParams::ENSPACE;
			else if (name == "enskip")
				isp.kind = InsetSpaceParams::ENSKIP;
			else if (name == "negthinspace")
				isp.kind = InsetSpaceParams::NEGTHIN;
			else if (name == "negmedspace")
				isp.kind = InsetSpaceParams::NEGMEDIUM;
			else if (name == "negthickspace")
				isp.kind = InsetSpaceParams::NEGTHICK;
			else if (name == "hfill")
				isp.kind = InsetSpaceParams::HFILL;
			else if (name == "hfill*")
				isp.kind = InsetSpaceParams::HFILL_PROTECTED;
			else if (name == "dotfill")
				isp.kind = InsetSpaceParams::DOTFILL;
			else if (name == "hrulefill")
				isp.kind = InsetSpaceParams::HRULEFILL;
			else if (name == "hspace") {
				if (len.empty() || !isValidGlueLength(len)) {
					lyxerr << "LyX function 'space-insert hspace' "
					       << "needs a valid length argument." << endl;
					break;
				}
				isp.kind = InsetSpaceParams::CUSTOM;
				isp.length = GlueLength(len);
			}
			else if (name == "hspace*") {
				if (len.empty() || !isValidGlueLength(len)) {
					lyxerr << "LyX function 'space-insert hspace*' "
					       << "needs a valid length argument." << endl;
					break;
				}
				isp.kind = InsetSpaceParams::CUSTOM_PROTECTED;
				isp.length = GlueLength(len);
			}
			else {
				lyxerr << "Wrong argument for LyX function 'space-insert'." << endl;
				break;
			}
			return new InsetSpace(isp);
		}
		break;

		default:
			break;
		}

	} catch (ExceptionMessage const & message) {
		if (message.type_ == ErrorException) {
			// This should never happen!
			Alert::error(message.title_, message.details_);
			lyx_exit(1);
		} else if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
			return 0;
		}
	}

	return 0;
}


Inset * createInset(Buffer * buf, FuncRequest const & cmd)
{
	Inset * inset = createInsetHelper(buf, cmd);
	if (inset)
		inset->setBuffer(*buf);
	return inset;
}


Inset * readInset(Lexer & lex, Buffer * buf)
{
	// consistency check
	if (lex.getString() != "\\begin_inset")
		LYXERR0("Buffer::readInset: Consistency check failed.");

	unique_ptr<Inset> inset;

	string tmptok;
	lex >> tmptok;

	// test the different insets
	
	// FIXME It would be better if we did not have this branch and could
	// just do one massive switch for all insets. But at present, it's
	// easier to do it this way, and we can't do the massive switch until
	// the conversion mentioned below.  Note that if we do want to do a
	// single switch, we need to remove this "CommandInset" line---or
	// replace it with a single "InsetType" line that would be used in all
	// insets.
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
		inscmd.Read(lex, buf);

		switch (code) {
			case BIBITEM_CODE:
				inset.reset(new InsetBibitem(buf, inscmd));
				break;
			case BIBTEX_CODE:
				inset.reset(new InsetBibtex(buf, inscmd));
				break;
			case CITE_CODE: 
				inset.reset(new InsetCitation(buf, inscmd));
				break;
			case HYPERLINK_CODE:
				inset.reset(new InsetHyperlink(buf, inscmd));
				break;
			case INCLUDE_CODE:
				inset.reset(new InsetInclude(buf, inscmd));
				break;
			case INDEX_PRINT_CODE:
				inset.reset(new InsetPrintIndex(buf, inscmd));
				break;
			case LABEL_CODE:
				inset.reset(new InsetLabel(buf, inscmd));
				break;
			case LINE_CODE:
				inset.reset(new InsetLine(buf, inscmd));
				break;
			case NOMENCL_CODE:
				inset.reset(new InsetNomencl(buf, inscmd));
				break;
			case NOMENCL_PRINT_CODE:
				inset.reset(new InsetPrintNomencl(buf, inscmd));
				break;
			case REF_CODE:
				if (inscmd["name"].empty() && inscmd["reference"].empty())
					return 0;
				inset.reset(new InsetRef(buf, inscmd));
				break;
			case TOC_CODE:
				inset.reset(new InsetTOC(buf, inscmd));
				break;
			case NO_CODE:
			default:
				lyxerr << "unknown CommandInset '" << insetType
							<< "'" << endl;
				while (lex.isOK() && lex.getString() != "\\end_inset")
					lex.next();
				return 0;
		}
		inset->setBuffer(*buf);
	} else { 
		// FIXME This branch should be made to use inset codes
		// as the preceding branch does. Unfortunately, that
		// will take some doing. It requires converting the
		// representation of the insets in LyX files so that
		// they use the inset names listed in Inset.cpp. Then,
		// as above, the inset names can be translated to
		// inset codes using insetCode(). And the insets'
		// write() routines should use insetName() rather than
		// hardcoding it.
		if (tmptok == "Quotes") {
			inset.reset(new InsetQuotes(buf));
		} else if (tmptok == "External") {
			inset.reset(new InsetExternal(buf));
		} else if (tmptok == "FormulaMacro") {
			inset.reset(new MathMacroTemplate(buf));
		} else if (tmptok == "Formula") {
			inset.reset(new InsetMathHull(buf));
		} else if (tmptok == "Graphics") {
			inset.reset(new InsetGraphics(buf));
		} else if (tmptok == "Note") {
			inset.reset(new InsetNote(buf, tmptok));
		} else if (tmptok == "Box") {
			inset.reset(new InsetBox(buf, tmptok));
		} else if (tmptok == "Flex") {
			lex.eatLine();
			string s = lex.getString();
			inset.reset(new InsetFlex(buf, s));
		} else if (tmptok == "Branch") {
			inset.reset(new InsetBranch(buf, InsetBranchParams()));
		} else if (tmptok == "Phantom") {
			inset.reset(new InsetPhantom(buf, tmptok));
		} else if (tmptok == "ERT") {
			inset.reset(new InsetERT(buf));
		} else if (tmptok == "listings") {
			inset.reset(new InsetListings(buf));
		} else if (tmptok == "script") {
			inset.reset(new InsetScript(buf));
		} else if (tmptok == "space") {
			inset.reset(new InsetSpace);
		} else if (tmptok == "Tabular") {
			inset.reset(new InsetTabular(buf));
		} else if (tmptok == "Text") {
			inset.reset(new InsetText(buf));
		} else if (tmptok == "VSpace") {
			inset.reset(new InsetVSpace);
		} else if (tmptok == "Foot") {
			inset.reset(new InsetFoot(buf));
		} else if (tmptok == "Marginal") {
			inset.reset(new InsetMarginal(buf));
		} else if (tmptok == "Newpage") {
			inset.reset(new InsetNewpage);
		} else if (tmptok == "Newline") {
			inset.reset(new InsetNewline);
		} else if (tmptok == "Separator") {
			inset.reset(new InsetSeparator);
		} else if (tmptok == "Argument") {
			inset.reset(new InsetArgument(buf, tmptok));
		} else if (tmptok == "Float") {
			inset.reset(new InsetFloat(buf, string()));
		} else if (tmptok == "Wrap") {
			lex.next();
			string tmptok = lex.getString();
			inset.reset(new InsetWrap(buf, tmptok));
		} else if (tmptok == "Caption") {
			lex.eatLine();
			string s = lex.getString();
			inset.reset(new InsetCaption(buf, s));
		} else if (tmptok == "Index") {
			inset.reset(new InsetIndex(buf, InsetIndexParams()));
		} else if (tmptok == "FloatList") {
			inset.reset(new InsetFloatList(buf));
		} else if (tmptok == "Info") {
			inset.reset(new InsetInfo(buf));
		} else if (tmptok == "IPA") {
			inset.reset(new InsetIPA(buf));
		} else if (tmptok == "IPADeco") {
			inset.reset(new InsetIPADeco(buf, tmptok));
		} else if (tmptok == "Preview") {
			inset.reset(new InsetPreview(buf));
		} else {
			lyxerr << "unknown Inset type '" << tmptok
			       << "'" << endl;
			while (lex.isOK() && lex.getString() != "\\end_inset")
				lex.next();
			return 0;
		}

		// Set the buffer reference for proper parsing of some insets
		// (InsetCollapsable for example)
		inset->setBuffer(*buf);
		inset->read(lex);
		// Set again the buffer for insets that are created inside this inset
		// (InsetMathHull for example).
		inset->setBuffer(*buf);
	}
	return inset.release();
}


} // namespace lyx
