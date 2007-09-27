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
#include "BufferView.h"
#include "BufferParams.h"
#include "debug.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "Color.h"
#include "Lexer.h"
#include "LyX.h"

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
#include "insets/InsetNomencl.h"
#include "insets/InsetLabel.h"
#include "insets/InsetLine.h"
#include "insets/InsetMarginal.h"
#include "insets/InsetNote.h"
#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetOptArg.h"
#include "insets/InsetPagebreak.h"
#include "insets/InsetRef.h"
#include "insets/InsetSpace.h"
#include "insets/InsetTabular.h"
#include "insets/InsetTOC.h"
#include "insets/InsetUrl.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

#include "mathed/MathMacroTemplate.h"
#include "mathed/InsetMathHull.h"

#include "frontends/alert.h"

#include "support/lstrings.h"
#include "support/ExceptionMessage.h"

#include <boost/assert.hpp>
#include <boost/current_function.hpp>

#include <sstream>

using std::auto_ptr;
using std::endl;
using std::string;


namespace lyx {

namespace Alert = frontend::Alert;

using support::compare_ascii_no_case;


Inset * createInset(BufferView * bv, FuncRequest const & cmd)
{
	BufferParams const & params = bv->buffer().params();

	try {

		switch (cmd.action) {
		case LFUN_HFILL_INSERT:
			return new InsetHFill;

		case LFUN_LINE_INSERT:
			return new InsetLine;

		case LFUN_PAGEBREAK_INSERT:
			return new InsetPagebreak;

		case LFUN_CLEARPAGE_INSERT:
			return new InsetClearPage;

		case LFUN_CLEARDOUBLEPAGE_INSERT:
			return new InsetClearDoublePage;

		case LFUN_FLEX_INSERT: {
			string s = cmd.getArg(0);
			TextClass tclass = params.getTextClass();
			InsetLayout il = tclass.insetlayout(from_utf8(s));
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
			return new InsetBibitem(InsetCommandParams("bibitem"));

		case LFUN_FLOAT_INSERT: {
			// check if the float type exists
			string const argument = to_utf8(cmd.argument());
			if (params.getTextClass().floats().typeExist(argument))
				return new InsetFloat(params, argument);
			lyxerr << "Non-existent float type: " << argument << endl;
			return 0;
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
			if (argument == "figure")
				return new InsetWrap(params, argument);
			lyxerr << "Non-existent wrapfig type: " << argument << endl;
			return 0;
		}

		case LFUN_INDEX_INSERT: {
			// Try and generate a valid index entry.
			InsetCommandParams icp("index");
			icp["name"] = cmd.argument().empty() ?
				bv->cursor().innerText()->getStringToIndex(bv->cursor()) :
				cmd.argument();
			return new InsetIndex(icp);
		}

		case LFUN_NOMENCL_INSERT: {
			InsetCommandParams icp("nomenclature");
			icp["symbol"] = cmd.argument().empty() ?
				bv->cursor().innerText()->getStringToIndex(bv->cursor()) :
				cmd.argument();
			return new InsetNomencl(icp);
		}

		case LFUN_TABULAR_INSERT: {
			if (cmd.argument().empty())
				return 0;
			std::istringstream ss(to_utf8(cmd.argument()));
			int r = 0, c = 0;
			ss >> r >> c;
			if (r <= 0)
				r = 2;
			if (c <= 0)
				c = 2;
			return new InsetTabular(bv->buffer(), r, c);
		}

		case LFUN_CAPTION_INSERT: {
			auto_ptr<InsetCaption> inset(new InsetCaption(params));
			inset->setAutoBreakRows(true);
			inset->setDrawFrame(true);
			inset->setFrameColor(Color::captionframe);
			return inset.release();
		}

		case LFUN_INDEX_PRINT:
			return new InsetPrintIndex(InsetCommandParams("index_print"));

		case LFUN_NOMENCL_PRINT:
			return new InsetPrintNomencl(InsetCommandParams("nomencl_print"));

		case LFUN_TOC_INSERT:
			return new InsetTOC(InsetCommandParams("toc"));

		case LFUN_ENVIRONMENT_INSERT:
			return new InsetEnvironment(params, cmd.argument());

#if 0
		case LFUN_LIST_INSERT:
			return new InsetList;

		case LFUN_THEOREM_INSERT:
			return new InsetTheorem;
#endif

		case LFUN_INSET_INSERT: {
			string const name = cmd.getArg(0);

			if (name == "bibitem") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetBibitem(icp);

			} else if (name == "bibtex") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetBibtex(icp);

			} else if (name == "citation") {
				InsetCommandParams icp("citation");
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetCitation(icp);

			} else if (name == "ert") {
				InsetCollapsable::CollapseStatus st;
				InsetERTMailer::string2params(to_utf8(cmd.argument()), st);
				return new InsetERT(params, st);

			} else if (name == "listings") {
				InsetListingsParams par;
				InsetListingsMailer::string2params(to_utf8(cmd.argument()), par);
				return new InsetListings(params, par);

			} else if (name == "external") {
				Buffer const & buffer = bv->buffer();
				InsetExternalParams iep;
				InsetExternalMailer::string2params(to_utf8(cmd.argument()),
					buffer, iep);
				auto_ptr<InsetExternal> inset(new InsetExternal);
				inset->setParams(iep, buffer);
				return inset.release();

			} else if (name == "graphics") {
				Buffer const & buffer = bv->buffer();
				InsetGraphicsParams igp;
				InsetGraphicsMailer::string2params(to_utf8(cmd.argument()),
					buffer, igp);
				auto_ptr<InsetGraphics> inset(new InsetGraphics);
				inset->setParams(igp);
				return inset.release();

			} else if (name == "include") {
				InsetCommandParams iip(name);
				InsetIncludeMailer::string2params(to_utf8(cmd.argument()), iip);
				return new InsetInclude(iip);

			} else if (name == "index") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetIndex(icp);

			} else if (name == "nomenclature") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, lyx::to_utf8(cmd.argument()),
					icp);
				return new InsetNomencl(icp);

			} else if (name == "label") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetLabel(icp);

			} else if (name == "ref") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetRef(icp, bv->buffer());

			} else if (name == "toc") {
				InsetCommandParams icp("toc");
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetTOC(icp);

			} else if (name == "url") {
				InsetCommandParams icp(name);
				InsetCommandMailer::string2params(name, to_utf8(cmd.argument()),
					icp);
				return new InsetUrl(icp);

			} else if (name == "vspace") {
				VSpace vspace;
				InsetVSpaceMailer::string2params(to_utf8(cmd.argument()), vspace);
				return new InsetVSpace(vspace);
			}
		}

		case LFUN_SPACE_INSERT: {
			string const name = to_utf8(cmd.argument());
			if (name == "normal")
				return new InsetSpace(InsetSpace::NORMAL);
			else if (name == "protected")
				return new InsetSpace(InsetSpace::PROTECTED);
			else if (name == "thin")
				return new InsetSpace(InsetSpace::THIN);
			else if (name == "quad")
				return new InsetSpace(InsetSpace::QUAD);
			else if (name == "qquad")
				return new InsetSpace(InsetSpace::QQUAD);
			else if (name == "enspace")
				return new InsetSpace(InsetSpace::ENSPACE);
			else if (name == "enskip")
				return new InsetSpace(InsetSpace::ENSKIP);
			else if (name == "negthinspace")
				return new InsetSpace(InsetSpace::NEGTHIN);
			else if (name.empty())
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

	TextClass tclass = buf.params().getTextClass();

	lex.next();
	string tmptok = lex.getString();

	// test the different insets
	if (tmptok == "CommandInset") {
		lex.next();
		string const insetType = lex.getString();
		lex.pushToken(insetType);

		//FIXME 
		//Inset::Code const code = Inset::translate(insetType);
		//if (code == Inset::NO_CODE) { choke as below; }
		//InsetCommandParams inscmd();
		InsetCommandParams inscmd(insetType);
		inscmd.read(lex);

		if (insetType == "citation") {
			inset.reset(new InsetCitation(inscmd));
		} else if (insetType == "bibitem") {
			inset.reset(new InsetBibitem(inscmd));
		} else if (insetType == "bibtex") {
			inset.reset(new InsetBibtex(inscmd));
		} else if (insetType == "index") {
			inset.reset(new InsetIndex(inscmd));
		} else if (insetType == "nomenclature") {
			inset.reset(new InsetNomencl(inscmd));
		} else if (insetType == "include") {
			inset.reset(new InsetInclude(inscmd));
		} else if (insetType == "label") {
			inset.reset(new InsetLabel(inscmd));
		} else if (insetType == "url") {
			inset.reset(new InsetUrl(inscmd));
		} else if (insetType == "ref") {
			if (!inscmd["name"].empty()
			    || !inscmd["reference"].empty()) {
				inset.reset(new InsetRef(inscmd, buf));
			}
		} else if (insetType == "toc") {
			inset.reset(new InsetTOC(inscmd));
		} else if (insetType == "index_print") {
			inset.reset(new InsetPrintIndex(inscmd));
		} else if (insetType == "nomencl_print") {
			inset.reset(new InsetPrintNomencl(inscmd));
		} else {
			lyxerr << "unknown CommandInset '" << insetType
			       << "'" << std::endl;
			while (lex.isOK() && lex.getString() != "\\end_inset")
				lex.next();
			return 0;
		}
	} else {
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
			InsetLayout il = tclass.insetlayout(from_utf8(s));
			inset.reset(new InsetFlex(buf.params(), il));
		} else if (tmptok == "Branch") {
			inset.reset(new InsetBranch(buf.params(),
						    InsetBranchParams()));
		} else if (tmptok == "Include") {
			InsetCommandParams p("include");
			inset.reset(new InsetInclude(p));
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
		} else if (tmptok == "List") {
			inset.reset(new InsetList);
		} else if (tmptok == "Theorem") {
			inset.reset(new InsetList);
#endif
		} else if (tmptok == "Caption") {
			inset.reset(new InsetCaption(buf.params()));
		} else if (tmptok == "FloatList") {
			inset.reset(new InsetFloatList);
		} else {
			lyxerr << "unknown Inset type '" << tmptok
			       << "'" << std::endl;
			while (lex.isOK() && lex.getString() != "\\end_inset")
				lex.next();
			return 0;
		}

		inset->read(buf, lex);

// FIXME: hack..
		if (inset->lyxCode() == Inset::MATHMACRO_CODE) {
			MathMacroTemplate const * tmpl =
				static_cast<MathMacroTemplate*>(inset.get());
			MacroTable::globalMacros().insert
				(tmpl->name(), tmpl->asMacroData());
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": creating local macro " << to_utf8(tmpl->name())
				<< endl;
		}
	}

	return inset.release();
}


} // namespace lyx
