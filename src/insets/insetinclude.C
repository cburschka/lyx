#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION 
#include "insetinclude.h"
#include "filedlg.h" 
#include "buffer.h"
#include "bufferlist.h"
#include "debug.h"
#include "support/filetools.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "LaTeXFeatures.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "gettext.h"
#include "include_form.h"
#include "support/FileInfo.h"
#include "layout.h"
#include "lyxfunc.h"

using std::ostream;
using std::endl;
using std::vector;
using std::pair;

extern BufferView * current_view;

extern BufferList bufferlist;


FD_include * create_form_include(void)
{
  FL_OBJECT * obj;
  FD_include * fdui = (FD_include *) fl_calloc(1, sizeof(FD_include));

  fdui->include = fl_bgn_form(FL_NO_BOX, 340, 210);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 210, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 70, 160, 90, "");
  fdui->browsebt = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 30, 100, 30, idex(_("Browse|#B")));
    fl_set_button_shortcut(obj, scex(_("Browse|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 0);
  fdui->flag1 = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 70, 150, 30, idex(_("Don't typeset|#D")));
    fl_set_button_shortcut(obj, scex(_("Don't typeset|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_button(FL_RETURN_BUTTON, 120, 170, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 1);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 170, 100, 30, idex(_("Cancel|^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 2);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 130, 100, 30, idex(_("Load|#L")));
    fl_set_button_shortcut(obj, scex(_("Load|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 5);
  fdui->input = obj = fl_add_input(FL_NORMAL_INPUT, 10, 30, 210, 30, idex(_("File name:|#F")));
    fl_set_input_shortcut(obj, scex(_("File name:|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->flag41 = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 100, 150, 30, idex(_("Visible space|#s")));
    fl_set_button_shortcut(obj, scex(_("Visible space|#s")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->include_grp = fl_bgn_group();
  fdui->flag4 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 130, 160, 30, idex(_("Verbatim|#V")));
    fl_set_button_shortcut(obj, scex(_("Verbatim|#V")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 10);
  fdui->flag2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 100, 160, 30, idex(_("Use input|#i")));
    fl_set_button_shortcut(obj, scex(_("Use input|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 11);
  fdui->flag3 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 70, 160, 30, idex(_("Use include|#U")));
    fl_set_button_shortcut(obj, scex(_("Use include|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, include_cb, 11);
  fl_end_group();

  fl_end_form();

  //fdui->include->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


FD_include * form = 0;

extern "C"
void include_cb(FL_OBJECT *, long arg)
{
    
	InsetInclude * inset = static_cast<InsetInclude*>(form->include->u_vdata);
	switch (arg) {
	case 0:
	{
		// Should browsing too be disabled in RO-mode?
		LyXFileDlg fileDlg;
		string mpath = OnlyPath(inset->getMasterFilename());
                string ext;
    
		if (fl_get_button(form->flag2)) // Use Input Button
			ext = "*.tex";
		else if (fl_get_button(form->flag4)) // Verbatim all files
			ext = "*";
                else
                        ext = "*.lyx";
		// launches dialog
		fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
    
		// Use by default the master's path
		string filename = fileDlg.Select(_("Select Child Document"),
						  mpath, ext, 
						  inset->getContents());
		XFlush(fl_get_display());
 
		// check selected filename
		if (!filename.empty()) {
			string filename2 = MakeRelPath(filename,
							mpath);
			if (prefixIs(filename2, ".."))
				fl_set_input(form->input,
					     filename.c_str());
			else
				fl_set_input(form->input,
					     filename2.c_str());
		}
		break;
	}

	case 1:
		if(!current_view->buffer()->isReadonly()) {
			inset->setContents(fl_get_input(form->input));
			// don't typeset
			inset->setNoLoad(fl_get_button(form->flag1));
			if (fl_get_button(form->flag2))
				inset->setInput();
			else if (fl_get_button(form->flag3))
				inset->setInclude();
			else if (fl_get_button(form->flag4)) {
				inset->setVerb();
				inset->setVisibleSpace(fl_get_button(form->flag41));
			}
			
			fl_hide_form(form->include);
			current_view->updateInset(inset, true);
			break;
		} // fall through
		
	case 2:
		fl_hide_form(form->include);
		break;
	case 5:
		if(!current_view->buffer()->isReadonly()) {
			inset->setContents(fl_get_input(form->input));
			inset->setNoLoad(fl_get_button(form->flag1));
			if (fl_get_button(form->flag2))
				inset->setInput();
			else if (fl_get_button(form->flag3))
				inset->setInclude();
			else if (fl_get_button(form->flag4)) {
				inset->setVerb();
				inset->setVisibleSpace(fl_get_button(form->flag41));
			}
			
			fl_hide_form(form->include);
			current_view->updateInset(inset, true);
			current_view->owner()->getLyXFunc()->Dispatch(LFUN_CHILDOPEN, inset->getContents().c_str());
                }
                break;
		
        case 10:
                fl_activate_object(form->flag41);
                fl_set_object_lcol(form->flag41, FL_BLACK); 
                break;
        case 11:
                fl_deactivate_object(form->flag41);
                fl_set_object_lcol(form->flag41, FL_INACTIVE);
	        fl_set_button(form->flag41, 0);
                break;
	}
}


static string unique_id() {
	static unsigned int seed=1000;

#ifdef HAVE_SSTREAM
	std::ostringstream ost;
	ost << "file" << ++seed;
#else
	char ctmp[16];
	ostrstream ost(ctmp,16);
	ost << "file" << ++seed << '\0';
#endif

	return ost.str();
}


InsetInclude::InsetInclude(string const & fname, Buffer * bf)
	: InsetCommand("include") 
{
	master = bf;
	setContents(fname);
	flag = InsetInclude::INCLUDE;
	noload = false;
	include_label = unique_id();
}


InsetInclude::~InsetInclude()
{
	if (form && form->include->u_vdata == this) {
		// this inset is in the popup so hide the popup 
		// and remove the reference to this inset. ARRae
		if (form->include) {
			if (form->include->visible) {
				fl_hide_form(form->include);
			}
			fl_free_form(form->include);
		}
		fl_free(form);
		form = 0;
	}
}


Inset * InsetInclude::Clone() const
{ 
	InsetInclude * ii = new InsetInclude (getContents(), master); 
	ii->setNoLoad(isNoLoad());
	// By default, the newly created inset is of `include' type,
	// so we do not test this case.
	if (isInput())
		ii->setInput();
	else if (isVerb()) {
		ii->setVerb();
		ii->setVisibleSpace(isVerbVisibleSpace());
	}
	return ii;
}


void InsetInclude::Edit(BufferView * bv, int, int, unsigned int)
{
	if(bv->buffer()->isReadonly())
		WarnReadonly(bv->buffer()->fileName());

	if (!form) {
                form = create_form_include();
		fl_set_form_atclose(form->include, IgnoreCloseBoxCB, 0);
	}
        form->include->u_vdata = this;
    
        fl_set_input(form->input, getContents().c_str());
	fl_set_button(form->flag1, int(isNoLoad()));
	fl_set_button(form->flag2, int(isInput()));
	fl_set_button(form->flag3, int(isInclude()));
	fl_set_button(form->flag4, int(isVerb()));
        if (isVerb()) 
            fl_set_button(form->flag41, int(isVerbVisibleSpace()));
        else {
	    fl_set_button(form->flag41, 0);
            fl_deactivate_object(form->flag41);
	    fl_set_object_lcol(form->flag41, FL_INACTIVE);
	}
	
        if (form->include->visible) {
		fl_raise_form(form->include);
	} else {
		fl_show_form(form->include, FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Include"));
	}
}


void InsetInclude::Write(Buffer const *, ostream & os) const
{
	os << "Include " << getCommand() << "\n";
}


void InsetInclude::Read(Buffer const * buf, LyXLex & lex)
{
	InsetCommand::Read(buf, lex);
    
	if (getCmdName() == "include")
		setInclude();
	else if (getCmdName() == "input")
		setInput();
	else if (contains(getCmdName(), "verbatim")) {
		setVerb();
		if (getCmdName() == "verbatiminput*")
			setVisibleSpace(true);
	}
}


bool InsetInclude::display() const 
{
	return !isInput();
}


string InsetInclude::getScreenLabel() const
{
	string temp;
	if (isInput())
		temp += _("Input");
	else if (isVerb()) {
		temp += _("Verbatim Input");
		if (isVerbVisibleSpace()) temp += '*';
	} else temp += _("Include");
	temp += ": ";
	
	if (getContents().empty()) {
		temp+= "???";
	} else {
		temp+= getContents();
	}
	return temp;
}


void InsetInclude::setContents(string const & c)
{
	InsetCommand::setContents(c);
	filename = MakeAbsPath(getContents(), 
			       OnlyPath(getMasterFilename())); 
}


bool InsetInclude::loadIfNeeded() const
{
	if (isNoLoad() || isVerb()) return false;
	if (!IsLyXFilename(getFileName())) return false;
	
	if (bufferlist.exists(getFileName())) return true;
	
	// the readonly flag can/will be wrong, not anymore I think.
	FileInfo finfo(getFileName());
	bool ro = !finfo.writable();
	return ( bufferlist.readFile(getFileName(), ro) != 0 );
}


int InsetInclude::Latex(Buffer const *, ostream & os,
			bool /*fragile*/, bool /*fs*/) const
{
	// Do nothing if no file name has been specified
	if (getContents().empty())
		return 0;
    
	// Use += to force a copy of contents (JMarc)
	// How does that force anything? (Lgb)
	string incfile(getContents());

	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		if (tmp->params.textclass != master->params.textclass) {
			lyxerr << "ERROR: Cannot handle include file `"
			       << MakeDisplayPath(getFileName())
			       << "' which has textclass `"
			       << textclasslist.NameOfClass(tmp->params.textclass)
			       << "' instead of `"
			       << textclasslist.NameOfClass(master->params.textclass)
			       << "'." << endl;
			return 0;
		}
		
		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".tex");
		if (!master->tmppath.empty()
		    && !master->niceFile) {
			incfile = subst(incfile, '/','@');
#ifdef __EMX__
			incfile = subst(incfile, ':', '$');
#endif
			writefile = AddName(master->tmppath, incfile);
		} else
			writefile = getFileName();
		writefile = ChangeExtension(writefile, ".tex");
		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->markDepClean(master->tmppath);
		
		tmp->makeLaTeXFile(writefile,
				   OnlyPath(getMasterFilename()), 
				   master->niceFile, true);
	} 

	if (isVerb()) {
		os << '\\' << getCmdName() << '{' << incfile << '}';
	} else if (isInput()) {
		// \input wants file with extension (default is .tex)
		if (!IsLyXFilename(getFileName())) {
			os << '\\' << getCmdName() << '{' << incfile << '}';
		} else {
			os << '\\' << getCmdName() << '{'
			   << ChangeExtension(incfile, ".tex")
			   <<  '}';
		}
	} else {
		// \include don't want extension and demands that the
		// file really have .tex
		os << '\\' << getCmdName() << '{'
		   << ChangeExtension(incfile, string())
		   << '}';
	}

	return 0;
}


int InsetInclude::Linuxdoc(Buffer const *, ostream & os) const
{
	// Do nothing if no file name has been specified
	if (getContents().empty())
		return 0;
    
	string incfile(getContents());

	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".sgml");
		if (!master->tmppath.empty() && !master->niceFile) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(master->tmppath, incfile);
		} else
			writefile = getFileName();

		if(IsLyXFilename(getFileName()))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->makeLinuxDocFile(writefile, master->niceFile, true);
	} 

	if (isVerb()) {
		os << "<!-- includefile verbatim=\"" << incfile << "\" -->";
	} else 
		os << '&' << include_label << ';';
	
	return 0;
}


int InsetInclude::DocBook(Buffer const *, ostream & os) const
{
	// Do nothing if no file name has been specified
	if (getContents().empty())
		return 0;
    
	string incfile(getContents());

	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".sgml");
		if (!master->tmppath.empty() && !master->niceFile) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(master->tmppath, incfile);
		} else
			writefile = getFileName();
		if(IsLyXFilename(getFileName()))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->makeDocBookFile(writefile, master->niceFile, true);
	} 

	if (isVerb()) {
		os << "<!-- includefile verbatim=\"" << incfile << "\" -->";
	} else 
		os << '&' << include_label << ';';
	
	return 0;
}


void InsetInclude::Validate(LaTeXFeatures & features) const
{

	string incfile(getContents());
	string writefile = ChangeExtension(getFileName(), ".sgml");
	if (!master->tmppath.empty() && !master->niceFile) {
		incfile = subst(incfile, '/','@');
		writefile = AddName(master->tmppath, incfile);
	} else
		// writefile = getFileName();
		// Use the relative path.
		writefile = incfile;

	if(IsLyXFilename(getFileName()))
		writefile = ChangeExtension(writefile, ".sgml");

	features.IncludedFiles[include_label] = writefile;

	if (isVerb())
		features.verbatim = true;

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded()) {
		// a file got loaded
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->validate(features);
	}
}


vector<string> InsetInclude::getLabelList() const
{
    vector<string> l;
    string parentname;

    if (loadIfNeeded()) {
	Buffer * tmp = bufferlist.getBuffer(getFileName());
	tmp->setParentName(""); 
	l = tmp->getLabelList();
	tmp->setParentName(getMasterFilename());
    }

    return l;
}


vector<pair<string,string> > InsetInclude::getKeys() const
{
	vector<pair<string,string> > keys;
	
	if (loadIfNeeded()) {
		Buffer *tmp = bufferlist.getBuffer(getFileName());
		tmp->setParentName(""); 
		keys =  tmp->getBibkeyList();
		tmp->setParentName(getMasterFilename());
	}
	
	return keys;
}
