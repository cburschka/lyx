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
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "lyxlex.h"
#include "metricsinfo.h"

#include "frontends/Painter.h"

#include "graphics/PreviewedInset.h"
#include "graphics/PreviewImage.h"

#include "support/FileInfo.h"
#include "support/FileMonitor.h"
#include "support/filetools.h"
#include "support/lstrings.h" // contains
#include "support/tostr.h"

#include <boost/bind.hpp>

#include "support/std_sstream.h"

using lyx::support::AddName;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::FileInfo;
using lyx::support::FileMonitor;
using lyx::support::GetFileContents;
using lyx::support::IsFileReadable;
using lyx::support::IsLyXFilename;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::subst;

using std::endl;

using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


extern BufferList bufferlist;


class InsetInclude::PreviewImpl : public lyx::graphics::PreviewedInset {
public:
	///
	PreviewImpl(InsetInclude & p) : PreviewedInset(p) {}

	///
	bool previewWanted(Buffer const &) const;
	///
	string const latexString(Buffer const &) const;
	///
	InsetInclude const & parent() const {
		return dynamic_cast<InsetInclude const &>(inset());
	}

	///
	bool monitoring() const { return monitor_.get(); }
	///
	void startMonitoring(string const & file);
	///
	void stopMonitoring() { monitor_.reset(); }

private:
	/// Invoked by monitor_ should the parent file change.
	void restartLoading();
	///
	boost::scoped_ptr<FileMonitor> monitor_;
};


namespace {

string const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + tostr(++seed);
}

} // namespace anon


InsetInclude::InsetInclude(InsetCommandParams const & p)
	: params_(p), include_label(uniqueID()),
	  preview_(new PreviewImpl(*this)),
	  set_label_(false)
{}


InsetInclude::InsetInclude(InsetInclude const & other)
	: InsetOld(other),
	  params_(other.params_),
	  include_label(other.include_label),
	  preview_(new PreviewImpl(*this)),
	  set_label_(other.set_label_)
{}


InsetInclude::~InsetInclude()
{
	InsetIncludeMailer mailer(*this);
	mailer.hideDialog();
}


dispatch_result InsetInclude::localDispatch(FuncRequest const & cmd)
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
		return InsetOld::localDispatch(cmd);
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

} // namespace anon


void InsetInclude::set(InsetCommandParams const & p, Buffer const & buffer)
{
	params_ = p;
	set_label_ = false;

	if (preview_->monitoring())
		preview_->stopMonitoring();

	if (lyx::graphics::PreviewedInset::activated() &&
	    type(params_) == INPUT)
		preview_->generatePreview(buffer);
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
	if (preview_->previewReady()) {
		dim.asc = preview_->pimage()->ascent();
		dim.des = preview_->pimage()->descent();
		dim.wid = preview_->pimage()->width();
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
	cache(pi.base.bv);
	if (!preview_->previewReady()) {
		button_.draw(pi, x + button_.box().x1, y);
		return;
	}

	if (!preview_->monitoring()) {
		string const included_file =
			includedFilename(*view()->buffer(), params_);
		preview_->startMonitoring(included_file);
	}

	pi.pain.image(x + button_.box().x1, y - dim_.asc, dim_.wid, dim_.height(),
			    *(preview_->pimage()->image()));
}


BufferView * InsetInclude::view() const
{
	return button_.view();
}


//
// preview stuff
//

void InsetInclude::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	preview_->addPreview(ploader);
}


bool InsetInclude::PreviewImpl::previewWanted(Buffer const & buffer) const
{
	string const included_file = includedFilename(buffer, parent().params());

	return type(parent().params_) == INPUT &&
		parent().params_.preview() &&
		IsFileReadable(included_file);
}


string const InsetInclude::PreviewImpl::latexString(Buffer const & buffer) const
{
	ostringstream os;
	LatexRunParams runparams;
	runparams.flavor = LatexRunParams::LATEX;
	parent().latex(buffer, os, runparams);

	return os.str();
}


void InsetInclude::PreviewImpl::startMonitoring(string const & file)
{
	monitor_.reset(new FileMonitor(file, 2000));
	monitor_->connect(boost::bind(&PreviewImpl::restartLoading, this));
	monitor_->start();
}


void InsetInclude::PreviewImpl::restartLoading()
{
	removePreview();
	if (!view())
		return;
	view()->updateInset(&parent());
	if (view()->buffer())
		generatePreview(*view()->buffer());
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
