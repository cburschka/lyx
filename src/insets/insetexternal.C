/**
 * \file insetexternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetexternal.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "ExternalTemplate.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "lyx_main.h"
#include "lyxlex.h"
#include "Lsstream.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/path.h"
#include "support/systemcall.h"
#include "support/FileInfo.h"

#include <cstdio>
#include <utility>

using std::ostream;
using std::endl;


InsetExternal::InsetExternal()
	: view_(0)
{
	tempname_ = lyx::tempName(string(), "lyxext");
	//ExternalTemplateManager::Templates::const_iterator i1;
	params_.templ = ExternalTemplateManager::get().getTemplates().begin()->second;
}


InsetExternal::~InsetExternal()
{
	lyx::unlink(tempname_);
	InsetExternalMailer mailer(*this);
	mailer.hideDialog();
}


InsetExternal::Params const & InsetExternal::params() const
{
	return params_;
}


dispatch_result InsetExternal::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, p);
		if (!p.filename.empty()) {
			setFromParams(p);
			cmd.view()->updateInset(this);
		}
		return DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetExternalMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
	case LFUN_INSET_EDIT:
		InsetExternalMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return UNDISPATCHED;
	}
}


void InsetExternal::setFromParams(Params const & p)
{
	params_.filename = p.filename;
	params_.parameters = p.parameters;
	params_.templ = p.templ;
}


string const InsetExternal::editMessage() const
{
	return doSubstitution(0, params_.templ.guiName);
}


void InsetExternal::write(Buffer const *, ostream & os) const
{
	os << "External " << params_.templ.lyxName << ",\""
	   << params_.filename << "\",\"" << params_.parameters << "\"\n";
}


void InsetExternal::read(Buffer const *, LyXLex & lex)
{
	string format;
	string token;

	// Read inset data from lex and store in format
	if (lex.eatLine()) {
		format = lex.getString();
	} else {
		lex.printError("InsetExternal: Parse error: `$$Token'");
	}

	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}

	// Parse string format...
	string::size_type const pos1 = format.find(',');
	params_.templ = ExternalTemplateManager::get().getTemplateByName(format.substr(0, pos1));
	string::size_type const pos2 = format.find("\",\"", pos1);
	params_.filename = format.substr(pos1 + 2, pos2 - (pos1 + 2));
	params_.parameters = format.substr(pos2 + 3, format.length() - (pos2 + 4));

	lyxerr[Debug::INFO] << "InsetExternal::Read: " << params_.templ.lyxName
			    << ' ' << params_.filename
			    << ' ' << params_.parameters << endl;
}


int InsetExternal::write(string const & format,
			 Buffer const * buf, ostream & os) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr << "External template format '" << format
		       << "' not specified in template "
		       << params_.templ.lyxName << endl;
		return 0;
	}

	updateExternal(format, buf);
	string const str = doSubstitution(buf, cit->second.product);
	os << str;
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetExternal::latex(Buffer const * buf, ostream & os,
			 LatexRunParams const & runparams) const
{
	// If the template has specified a PDFLaTeX output, then we try and
	// use that.
	if (runparams.flavor == LatexRunParams::PDFLATEX) {
		ExternalTemplate const & et = params_.templ;
		ExternalTemplate::Formats::const_iterator cit =
			et.formats.find("PDFLaTeX");
		if (cit != et.formats.end())
			return write("PDFLaTeX", buf, os);
	}

	return write("LaTeX", buf, os);
}


int InsetExternal::ascii(Buffer const * buf, ostream & os, int) const
{
	return write("Ascii", buf, os);
}


int InsetExternal::linuxdoc(Buffer const * buf, ostream & os) const
{
	return write("LinuxDoc", buf, os);
}


int InsetExternal::docbook(Buffer const * buf, ostream & os, bool) const
{
	return write("DocBook", buf, os);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");

	if (cit == et.formats.end())
		return;

	if (!cit->second.requirement.empty()) {
		features.require(cit->second.requirement);
	}
	if (!cit->second.preamble.empty()) {
		features.addExternalPreamble(cit->second.preamble + "\n");
	}
}


Inset * InsetExternal::clone() const
{
	InsetExternal * inset = new InsetExternal;
	inset->params_ = params_;
	inset->view_ = view_;
	return inset;
}


string const InsetExternal::getScreenLabel(Buffer const *) const
{
	ExternalTemplate const & et = params_.templ;
	if (et.guiName.empty())
		return _("External");
	else
		return doSubstitution(0, et.guiName);
}


void InsetExternal::executeCommand(string const & s,
				   Buffer const * buffer) const
{
	Path p(buffer->filePath());
	Systemcall one;
	if (lyxerr.debugging()) {
		lyxerr << "Executing '" << s << "' in '"
		       << buffer->filePath() << '\'' << endl;
	}
	one.startscript(Systemcall::Wait, s);
}


string const InsetExternal::doSubstitution(Buffer const * buffer,
					   string const & s) const
{
	string result;
	string const basename = ChangeExtension(params_.filename, string());
	string filepath;
	if (buffer && !buffer->tmppath.empty() && !buffer->niceFile) {
		filepath = buffer->filePath();
	}
	result = subst(s, "$$FName", params_.filename);
	result = subst(result, "$$Basename", basename);
	result = subst(result, "$$Parameters", params_.parameters);
	result = subst(result, "$$FPath", filepath);
	result = subst(result, "$$Tempname", tempname_);
	result = subst(result, "$$Sysdir", system_lyxdir);

	// Handle the $$Contents(filename) syntax
	if (contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;
		if (buffer) {
			// Make sure we are in the directory of the buffer
			Path p(buffer->filePath());
			contents = GetFileContents(file);
		} else {
			contents = GetFileContents(file);
		}
		result = subst(result,
			       ("$$Contents(\"" + file + "\")").c_str(),
			       contents);
	}

	return result;
}


void InsetExternal::updateExternal() const
{
	updateExternal("LaTeX", view_->buffer());
}

void InsetExternal::updateExternal(string const & format,
				   Buffer const * buf) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);

	if (cit == et.formats.end() ||
	    cit->second.updateCommand.empty() ||
	    !et.automaticProduction)
		return;

	if (!cit->second.updateResult.empty()) {
		string const resultfile = doSubstitution(buf,
							 cit->second.updateResult);
		FileInfo fi(params_.filename);
		FileInfo fi2(resultfile);
		if (fi2.exist() && fi.exist() &&
		    difftime(fi2.getModificationTime(),
			     fi.getModificationTime()) >= 0) {
			lyxerr[Debug::FILES] << resultfile
					     << " is up to date" << endl;
			return;
		}
	}

	executeCommand(doSubstitution(buf, cit->second.updateCommand), buf);
}


void InsetExternal::viewExternal() const
{
	ExternalTemplate const & et = params_.templ;
	if (et.viewCommand.empty())
		return;

	updateExternal();
	executeCommand(doSubstitution(view_->buffer(),
				      et.viewCommand),
		       view_->buffer());
}


void InsetExternal::editExternal() const
{
	ExternalTemplate const & et = params_.templ;
	if (et.editCommand.empty())
		return;

	updateExternal();
	executeCommand(doSubstitution(view_->buffer(),
				      et.editCommand),
		       view_->buffer());
}


string const InsetExternalMailer::name_("external");

InsetExternalMailer::InsetExternalMailer(InsetExternal & inset)
	: inset_(inset)
{}


string const InsetExternalMailer::inset2string() const
{
	return params2string(inset_.params());
}


void InsetExternalMailer::string2params(string const & in,
					InsetExternal::Params & params)
{
	params = InsetExternal::Params();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "External")
			return;
	}

	if (lex.isOK()) {
		InsetExternal inset;
		inset.read(0, lex);
		params = inset.params();
	}
}


string const
InsetExternalMailer::params2string(InsetExternal::Params const & params)
{
	InsetExternal inset;
	inset.setFromParams(params);
	ostringstream data;
	data << name_ << ' ';
	inset.write(0, data);
	data << "\\end_inset\n";
	return STRCONV(data.str());
}
