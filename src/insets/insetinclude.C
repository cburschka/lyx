/**
 * \file insetinclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetinclude.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "lyx_main.h"
#include "lyxlex.h"
#include "metricsinfo.h"

#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "graphics/PreviewLoader.h"

#include "insets/render_preview.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lstrings.h" // contains
#include "support/tostr.h"

#include <boost/bind.hpp>

#include "support/std_ostream.h"
#include "support/std_sstream.h"

using lyx::support::AddName;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::FileInfo;
using lyx::support::GetFileContents;
using lyx::support::IsFileReadable;
using lyx::support::IsLyXFilename;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::subst;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


extern BufferList bufferlist;


namespace {

string const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + tostr(++seed);
}

} // namespace anon


InsetInclude::InsetInclude(InsetCommandParams const & p)
	: params_(p), include_label(uniqueID()),
	  preview_(new RenderMonitoredPreview),
	  set_label_(false)
{
	preview_->connect(boost::bind(&InsetInclude::statusChanged, this));
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: InsetOld(other),
	  params_(other.params_),
	  include_label(other.include_label),
	  preview_(new RenderMonitoredPreview),
	  set_label_(other.set_label_)
{
	preview_->connect(boost::bind(&InsetInclude::statusChanged, this));
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::~InsetInclude()
{
	InsetIncludeMailer mailer(*this);
	mailer.hideDialog();
}


DispatchResult
InsetInclude::priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetIncludeMailer::string2params(cmd.argument, p);
		if (!p.getCmdName().empty()) {
			set(p, *cmd.view()->buffer());
			cmd.view()->updateInset(this);
		}
		return DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetIncludeMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
		if (button_.box().contains(cmd.x, cmd.y))
			InsetIncludeMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	case LFUN_INSET_DIALOG_SHOW:
		InsetIncludeMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return UNDISPATCHED;
	}
}


InsetCommandParams const & InsetInclude::params() const
{
	return params_;
}


namespace {

/// the type of inclusion
enum Types {
	INCLUDE = 0,
	VERB = 1,
	INPUT = 2,
	VERBAST = 3
};


Types type(InsetCommandParams const & params)
{
	string const command_name = params.getCmdName();

	if (command_name == "input")
		return INPUT;
	if  (command_name == "verbatiminput")
		return VERB;
	if  (command_name == "verbatiminput*")
		return VERBAST;
	return INCLUDE;
}


bool isVerbatim(InsetCommandParams const & params)
{
	string const command_name = params.getCmdName();
	return command_name == "verbatiminput" ||
		command_name == "verbatiminput*";
}


string const masterFilename(Buffer const & buffer)
{
	return buffer.fileName();
}


string const includedFilename(Buffer const & buffer,
			      InsetCommandParams const & params)
{
	return MakeAbsPath(params.getContents(),
			   OnlyPath(masterFilename(buffer)));
}


void add_preview(RenderMonitoredPreview &, InsetInclude const &, Buffer const &);

} // namespace anon


void InsetInclude::set(InsetCommandParams const & p, Buffer const & buffer)
{
	params_ = p;
	set_label_ = false;

	if (preview_->monitoring())
		preview_->stopMonitoring();

	if (type(params_) == INPUT)
		add_preview(*preview_, *this, buffer);
}


auto_ptr<InsetBase> InsetInclude::clone() const
{
	return auto_ptr<InsetBase>(new InsetInclude(*this));
}


void InsetInclude::write(Buffer const &, ostream & os) const
{
	write(os);
}


void InsetInclude::write(ostream & os) const
{
	os << "Include " << params_.getCommand() << '\n'
	   << "preview " << tostr(params_.preview()) << '\n';
}


void InsetInclude::read(Buffer const &, LyXLex & lex)
{
	read(lex);
}


void InsetInclude::read(LyXLex & lex)
{
	params_.read(lex);
}


string const InsetInclude::getScreenLabel(Buffer const &) const
{
	string temp;

	switch (type(params_)) {
		case INPUT: temp += _("Input"); break;
		case VERB: temp += _("Verbatim Input"); break;
		case VERBAST: temp += _("Verbatim Input*"); break;
		case INCLUDE: temp += _("Include"); break;
	}

	temp += ": ";

	if (params_.getContents().empty())
		temp += "???";
	else
		temp += OnlyFilename(params_.getContents());

	return temp;
}


namespace {

/// return true if the file is or got loaded.
bool loadIfNeeded(Buffer const & buffer, InsetCommandParams const & params)
{
	if (isVerbatim(params))
		return false;

	string const included_file = includedFilename(buffer, params);
	if (!IsLyXFilename(included_file))
		return false;

	if (bufferlist.exists(included_file))
		return true;

	// the readonly flag can/will be wrong, not anymore I think.
	FileInfo finfo(included_file);
	if (!finfo.isOK())
		return false;
	return loadLyXFile(bufferlist.newBuffer(included_file),
			   included_file);
}


} // namespace anon


int InsetInclude::latex(Buffer const & buffer, ostream & os,
			LatexRunParams const & runparams) const
{
	string incfile(params_.getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_);

	if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = bufferlist.getBuffer(included_file);

		// FIXME: this should be a GUI warning
		if (tmp->params().textclass != buffer.params().textclass) {
			lyxerr << "WARNING: Included file `"
			       << MakeDisplayPath(included_file)
			       << "' has textclass `"
			       << tmp->params().getLyXTextClass().name()
			       << "' while parent file has textclass `"
			       << buffer.params().getLyXTextClass().name()
			       << "'." << endl;
			//return 0;
		}

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(included_file, ".tex");

		if (!buffer.temppath().empty() && !runparams.nice) {
			incfile = subst(incfile, '/','@');
#ifdef __EMX__
			incfile = subst(incfile, ':', '$');
#endif
			writefile = AddName(buffer.temppath(), incfile);
		} else
			writefile = included_file;
		writefile = ChangeExtension(writefile, ".tex");
		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;

		tmp->markDepClean(buffer.temppath());

		tmp->makeLaTeXFile(writefile,
				   OnlyPath(masterFilename(buffer)),
				   runparams, false);
	}

	if (isVerbatim(params_)) {
		os << '\\' << params_.getCmdName() << '{' << incfile << '}';
	} else if (type(params_) == INPUT) {
		// \input wants file with extension (default is .tex)
		if (!IsLyXFilename(included_file)) {
			os << '\\' << params_.getCmdName() << '{' << incfile << '}';
		} else {
			os << '\\' << params_.getCmdName() << '{'
			   << ChangeExtension(incfile, ".tex")
			   <<  '}';
		}
	} else {
		// \include don't want extension and demands that the
		// file really have .tex
		os << '\\' << params_.getCmdName() << '{'
		   << ChangeExtension(incfile, string())
		   << '}';
	}

	return 0;
}


int InsetInclude::ascii(Buffer const & buffer, ostream & os, int) const
{
	if (isVerbatim(params_))
		os << GetFileContents(includedFilename(buffer, params_));
	return 0;
}


int InsetInclude::linuxdoc(Buffer const & buffer, ostream & os) const
{
	string incfile(params_.getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_);

	if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = bufferlist.getBuffer(included_file);

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(included_file, ".sgml");
		if (!buffer.temppath().empty() && !buffer.niceFile()) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(buffer.temppath(), incfile);
		} else
			writefile = included_file;

		if (IsLyXFilename(included_file))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;

		tmp->makeLinuxDocFile(writefile, buffer.niceFile(), true);
	}

	if (isVerbatim(params_)) {
		os << "<![CDATA["
		   << GetFileContents(included_file)
		   << "]]>";
	} else
		os << '&' << include_label << ';';

	return 0;
}


int InsetInclude::docbook(Buffer const & buffer, ostream & os,
			  bool /*mixcont*/) const
{
	string incfile(params_.getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_);

	if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = bufferlist.getBuffer(included_file);

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(included_file, ".sgml");
		if (!buffer.temppath().empty() && !buffer.niceFile()) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(buffer.temppath(), incfile);
		} else
			writefile = included_file;
		if (IsLyXFilename(included_file))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;

		tmp->makeDocBookFile(writefile, buffer.niceFile(), true);
	}

	if (isVerbatim(params_)) {
		os << "<inlinegraphic fileref=\""
		   << '&' << include_label << ';'
		   << "\" format=\"linespecific\">";
	} else
		os << '&' << include_label << ';';

	return 0;
}


void InsetInclude::validate(LaTeXFeatures & features) const
{
	string incfile(params_.getContents());
	string writefile;

	Buffer const & buffer = features.buffer();

	string const included_file = includedFilename(buffer, params_);

	if (!buffer.temppath().empty() &&
	    !buffer.niceFile() &&
	    !isVerbatim(params_)) {
		incfile = subst(incfile, '/','@');
		writefile = AddName(buffer.temppath(), incfile);
	} else
		writefile = included_file;

	if (IsLyXFilename(included_file))
		writefile = ChangeExtension(writefile, ".sgml");

	features.includeFile(include_label, writefile);

	if (isVerbatim(params_))
		features.require("verbatim");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded(buffer, params_)) {
		// a file got loaded
		Buffer * const tmp = bufferlist.getBuffer(included_file);
		if (tmp) {
			tmp->niceFile() = buffer.niceFile();
			tmp->validate(features);
		}
	}
}


void InsetInclude::getLabelList(Buffer const & buffer,
				std::vector<string> & list) const
{
	if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_);
		Buffer * tmp = bufferlist.getBuffer(included_file);
		tmp->setParentName("");
		tmp->getLabelList(list);
		tmp->setParentName(masterFilename(buffer));
	}
}


void InsetInclude::fillWithBibKeys(Buffer const & buffer,
				   std::vector<std::pair<string,string> > & keys) const
{
	if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_);
		Buffer * tmp = bufferlist.getBuffer(included_file);
		tmp->setParentName("");
		tmp->fillWithBibKeys(keys);
		tmp->setParentName(masterFilename(buffer));
	}
}


void InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (RenderPreview::activated() && preview_->previewReady()) {
		preview_->metrics(mi, dim);
	} else {
		if (!set_label_) {
			set_label_ = true;
			button_.update(getScreenLabel(*mi.base.bv->buffer()),
				       editable() != NOT_EDITABLE);
		}
		button_.metrics(mi, dim);
	}
	int center_indent = type(params_) == INPUT ?
		0 : (mi.base.textwidth - dim.wid) / 2;
	Box b(center_indent, center_indent + dim.wid, -dim.asc, dim.des);
	button_.setBox(b);

	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetInclude::draw(PainterInfo & pi, int x, int y) const
{
	if (!RenderPreview::activated() || !preview_->previewReady()) {
		button_.draw(pi, x + button_.box().x1, y);
		return;
	}

	preview_->draw(pi, x + button_.box().x1, y);
}


//
// preview stuff
//

void InsetInclude::statusChanged() const
{
	LyX::cref().updateInset(this);
}


void InsetInclude::fileChanged() const
{
	Buffer const * const buffer_ptr = LyX::cref().updateInset(this);
	if (!buffer_ptr)
		return;

	Buffer const & buffer = *buffer_ptr;
	preview_->removePreview(buffer);
	add_preview(*preview_.get(), *this, buffer);
	preview_->startLoading(buffer);
}


namespace {

bool preview_wanted(InsetCommandParams const & params, Buffer const & buffer)
{
	string const included_file = includedFilename(buffer, params);

	return type(params) == INPUT && params.preview() &&
		IsFileReadable(included_file);
}


string const latex_string(InsetInclude const & inset, Buffer const & buffer)
{
	ostringstream os;
	LatexRunParams runparams;
	runparams.flavor = LatexRunParams::LATEX;
	inset.latex(buffer, os, runparams);

	return os.str();
}


void add_preview(RenderMonitoredPreview & renderer, InsetInclude const & inset,
		 Buffer const & buffer)
{
	InsetCommandParams const & params = inset.params();
	if (RenderPreview::activated() && preview_wanted(params, buffer)) {
		renderer.setAbsFile(includedFilename(buffer, params));
		string const snippet = latex_string(inset, buffer);
		renderer.addPreview(snippet, buffer);
	}
}

} // namespace anon


void InsetInclude::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	Buffer const & buffer = ploader.buffer();
	if (preview_wanted(params(), buffer)) {
		preview_->setAbsFile(includedFilename(buffer, params()));
		string const snippet = latex_string(*this, buffer);
		preview_->addPreview(snippet, ploader);
	}
}


string const InsetIncludeMailer::name_("include");

InsetIncludeMailer::InsetIncludeMailer(InsetInclude & inset)
	: inset_(inset)
{}


string const InsetIncludeMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetIncludeMailer::string2params(string const & in,
				       InsetCommandParams & params)
{
	params = InsetCommandParams();

	if (in.empty())
		return;

	istringstream data(in);
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
		if (token != "Include")
			return;
	}

	if (lex.isOK()) {
		InsetInclude inset(params);
		inset.read(lex);
		params = inset.params();
	}
}


string const
InsetIncludeMailer::params2string(InsetCommandParams const & params)
{
	InsetInclude inset(params);
	ostringstream data;
	data << name_ << ' ';
	inset.write(data);
	data << "\\end_inset\n";
	return data.str();
}
