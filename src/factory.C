/**
 * \file factory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "factory.h"

#include "buffer.h"
#include "BufferView.h"
#include "bufferparams.h"
#include "debug.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "LColor.h"
#include "lyxlex.h"
#include "paragraph.h"

#include "insets/insetbibitem.h"
#include "insets/insetbibtex.h"
#include "insets/insetcaption.h"
#include "insets/insetcite.h"
#include "insets/insetcharstyle.h"
#include "insets/insetenv.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetfloatlist.h"
#include "insets/insetfoot.h"
#include "insets/insetgraphics.h"
#include "insets/insethfill.h"
#include "insets/insetinclude.h"
#include "insets/insetindex.h"
#include "insets/insetlabel.h"
#include "insets/insetline.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetnote.h"
#include "insets/insetbox.h"
#include "insets/insetbranch.h"
#include "insets/insetoptarg.h"
#include "insets/insetpagebreak.h"
#include "insets/insetref.h"
#include "insets/insetspace.h"
#include "insets/insettabular.h"
#include "insets/insettoc.h"
#include "insets/inseturl.h"
#include "insets/insetvspace.h"
#include "insets/insetwrap.h"
#include "mathed/formulamacro.h"
#include "mathed/formula.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "support/lstrings.h"
#include "support/std_sstream.h"

#include <boost/assert.hpp>

using lyx::support::compare_ascii_no_case;

using std::auto_ptr;
using std::endl;
using std::string;


InsetOld * createInset(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	BufferParams const & params = bv->buffer()->params();

	switch (cmd.action) {
	case LFUN_HFILL:
		return new InsetHFill;

	case LFUN_INSERT_LINE:
		return new InsetLine;

	case LFUN_INSERT_PAGEBREAK:
		return new InsetPagebreak;

	case LFUN_INSET_MINIPAGE:
		return new InsetMinipage(params);

	case LFUN_INSERT_CHARSTYLE: {
		string s = cmd.getArg(0);
		CharStyles::iterator found_cs = params.getLyXTextClass().charstyle(s);
		return new InsetCharStyle(params, found_cs);
	}

	case LFUN_INSERT_NOTE: {
		string arg = cmd.getArg(0);
		if (arg.empty())
			arg = "Note";
		return new InsetNote(params, arg);
	}

	case LFUN_INSERT_BOX: {
		string arg = cmd.getArg(0);
		if (arg.empty())
			arg = "Boxed";
		return new InsetBox(params, arg);
	}

	case LFUN_INSERT_BRANCH: {
		string arg = cmd.getArg(0);
		if (arg.empty())
			arg = "none";
		return new InsetBranch(params, arg);
	}

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
			auto_ptr<InsetFloat> p(new InsetFloat(params, cmd.argument));
			p->wide(true, params);
			return p.release();
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
			bv->getLyXText()->getStringToIndex() :
			cmd.argument;
		icp.setContents(contents);

		string data = InsetCommandMailer::params2string("index", icp);
		LyXView * lv = bv->owner();

		if (icp.getContents().empty()) {
			lv->getDialogs().show("index", data, 0);
		} else {
			lv->dispatch(FuncRequest(bv, LFUN_INSET_APPLY, data));
		}
		return 0;
	}

	case LFUN_TABULAR_INSERT:
		if (!cmd.argument.empty()) {
			std::istringstream ss(cmd.argument);
			int r = 0, c = 0;
			ss >> r >> c;
			if (r <= 0) r = 2;
			if (c <= 0) c = 2;
			return new InsetTabular(*bv->buffer(), r, c);
		}
		bv->owner()->getDialogs().show("tabularcreate");
		return 0;

	case LFUN_INSET_CAPTION: 
	if (!bv->innerInset()) {
		auto_ptr<InsetCaption> inset(new InsetCaption(params));
		inset->setOwner(bv->innerInset());
		inset->setAutoBreakRows(true);
		inset->setDrawFrame(InsetText::LOCKED);
		inset->setFrameColor(LColor::captionframe);
		return inset.release();
	}
	return 0;

	case LFUN_INDEX_PRINT:
		return new InsetPrintIndex(InsetCommandParams("printindex"));

	case LFUN_TOC_INSERT:
		return new InsetTOC(InsetCommandParams("tableofcontents"));

	case LFUN_ENVIRONMENT_INSERT:
		return new InsetEnvironment(params, cmd.argument);

#if 0
	case LFUN_INSET_LIST:
		return new InsetList;

	case LFUN_INSET_THEOREM:
		return new InsetTheorem;
#endif

	case LFUN_INSET_INSERT: {
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
			return new InsetCitation(icp);

		} else if (name == "ert") {
			InsetERT::ERTStatus s;
			InsetERTMailer::string2params(cmd.argument, s);
			return new InsetERT(params, s);

		} else if (name == "external") {
			Buffer const & buffer = *cmd.view()->buffer();
			InsetExternalParams iep;
			InsetExternalMailer::string2params(cmd.argument,
							   buffer, iep);
			auto_ptr<InsetExternal> inset(new InsetExternal);
			inset->setParams(iep, buffer);
			return inset.release();

		} else if (name == "graphics") {
			Buffer const & buffer = *cmd.view()->buffer();
			InsetGraphicsParams igp;
			InsetGraphicsMailer::string2params(cmd.argument,
							   buffer, igp);
			auto_ptr<InsetGraphics> inset(new InsetGraphics);
			inset->setParams(igp);
			return inset.release();

		} else if (name == "include") {
			InsetCommandParams iip;
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

		} else if (name == "vspace") {
			VSpace vspace;
			InsetVSpaceMailer::string2params(cmd.argument, vspace);
			return new InsetVSpace(vspace);
		}
	}

	case LFUN_SPACE_INSERT: {
		string const name = cmd.argument;
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

	return 0;
}


InsetOld * readInset(LyXLex & lex, Buffer const & buf)
{
	// consistency check
	if (lex.getString() != "\\begin_inset") {
		lyxerr << "Buffer::readInset: Consistency check failed."
		       << endl;
	}

	auto_ptr<InsetOld> inset;

	LyXTextClass tclass = buf.params().getLyXTextClass();
		
	lex.next();
	string tmptok = lex.getString();

	// test the different insets
	if (tmptok == "LatexCommand") {
		InsetCommandParams inscmd;
		inscmd.read(lex);

		string const cmdName = inscmd.getCmdName();

		// This strange command allows LyX to recognize "natbib" style
		// citations: citet, citep, Citet etc.
		if (compare_ascii_no_case(cmdName.substr(0,4), "cite") == 0) {
			inset.reset(new InsetCitation(inscmd));
		} else if (cmdName == "bibitem") {
			lex.printError("Wrong place for bibitem");
			inset.reset(new InsetBibitem(inscmd));
		} else if (cmdName == "bibtex") {
			inset.reset(new InsetBibtex(inscmd));
		} else if (cmdName == "index") {
			inset.reset(new InsetIndex(inscmd));
		} else if (cmdName == "include") {
			inset.reset(new InsetInclude(inscmd));
		} else if (cmdName == "label") {
			inset.reset(new InsetLabel(inscmd));
		} else if (cmdName == "url"
			   || cmdName == "htmlurl") {
			inset.reset(new InsetUrl(inscmd));
		} else if (cmdName == "ref"
			   || cmdName == "eqref"
			   || cmdName == "pageref"
			   || cmdName == "vref"
			   || cmdName == "vpageref"
			   || cmdName == "prettyref") {
			if (!inscmd.getOptions().empty()
			    || !inscmd.getContents().empty()) {
				inset.reset(new InsetRef(inscmd, buf));
			}
		} else if (cmdName == "tableofcontents") {
			inset.reset(new InsetTOC(inscmd));
		} else if (cmdName == "listofalgorithms") {
			inset.reset(new InsetFloatList("algorithm"));
		} else if (cmdName == "listoffigures") {
			inset.reset(new InsetFloatList("figure"));
		} else if (cmdName == "listoftables") {
			inset.reset(new InsetFloatList("table"));
		} else if (cmdName == "printindex") {
			inset.reset(new InsetPrintIndex(inscmd));
		} else {
			lyxerr << "unknown CommandInset '" << cmdName
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
			inset.reset(new InsetFormulaMacro);
		} else if (tmptok == "Formula") {
			inset.reset(new InsetFormula);
		} else if (tmptok == "Graphics") {
			inset.reset(new InsetGraphics);
		} else if (tmptok == "Note"	|| tmptok == "Comment"
				|| tmptok == "Greyedout") {
			inset.reset(new InsetNote(buf.params(), tmptok));
		} else if (tmptok == "Boxed" || tmptok == "ovalbox"
		        || tmptok == "Shadowbox" || tmptok == "Doublebox"
		        || tmptok == "Ovalbox" || tmptok == "Frameless") {
			inset.reset(new InsetBox(buf.params(), tmptok));
		} else if (tmptok == "CharStyle") {
			lex.next();
			string s = lex.getString();
			CharStyles::iterator found_cs = tclass.charstyle(s);
			inset.reset(new InsetCharStyle(buf.params(), found_cs));
		} else if (tmptok == "Branch") {
			inset.reset(new InsetBranch(buf.params(), string()));
		} else if (tmptok == "Include") {
			InsetCommandParams p("Include");
			inset.reset(new InsetInclude(p));
		} else if (tmptok == "Environment") {
			lex.next();
			inset.reset(new InsetEnvironment(buf.params(), lex.getString()));
		} else if (tmptok == "ERT") {
			inset.reset(new InsetERT(buf.params()));
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
		} else if (tmptok == "Minipage") {
			inset.reset(new InsetMinipage(buf.params()));
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
	}

	return inset.release();
}
