#include <config.h>

#include <fstream>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION  
#include "insetbib.h"
#include "combox.h"
#include "buffer.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "gettext.h"
#include "bibforms.h"
#include "lyxtext.h"
#include "support/filetools.h"
#include "support/path.h"

using std::ostream;
using std::ifstream;
using std::getline;
using std::endl;
using std::vector;
using std::pair;

extern BufferView * current_view;

FD_bibitem_form * bibitem_form = 0;

FD_bibitem_form * create_form_bibitem_form(void);


extern "C"
void bibitem_cb(FL_OBJECT *, long data)
{
	InsetBibKey::Holder * holder =
		static_cast<InsetBibKey::Holder*>
		(bibitem_form->bibitem_form->u_vdata);
	
	holder->inset->callback( bibitem_form, data );
}


FD_bibitem_form * create_form_bibitem_form(void)
{
	FL_OBJECT * obj;
	FD_bibitem_form * fdui = (FD_bibitem_form *) fl_calloc(1, sizeof(FD_bibitem_form));

	fdui->bibitem_form = fl_bgn_form(FL_NO_BOX, 220, 130);
	obj = fl_add_box(FL_UP_BOX, 0, 0, 220, 130, "");
	fdui->key = obj = fl_add_input(FL_NORMAL_INPUT, 80, 10, 130, 30, idex(_("Key:|#K")));
	fl_set_input_shortcut(obj, scex(_("Key:|#K")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	obj = fl_add_button(FL_RETURN_BUTTON, 20, 90, 90, 30, _("OK"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, bibitem_cb, 1);
	obj = fl_add_button(FL_NORMAL_BUTTON, 120, 90, 90, 30, idex(_("Cancel|^[")));
	fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, bibitem_cb, 0);
	fdui->label = obj = fl_add_input(FL_NORMAL_INPUT, 80, 50, 130, 30, idex(_("Label:|#L")));
	fl_set_input_shortcut(obj, scex(_("Label:|#L")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_end_form();

	//fdui->bibitem_form->fdui = fdui;

	return fdui;
}


InsetBibKey::InsetBibKey(string const & key, string const & label)
	: InsetCommand("bibitem", key, label)
{
	counter = 1;
	if (key.empty())
		setCmdName(" ");
}


InsetBibKey::InsetBibKey(InsetBibKey const * b)
	: InsetCommand("bibitem", b->getContents(), b->getOptions())
{
	counter = b->counter;
}


InsetBibKey::~InsetBibKey()
{
	if(bibitem_form && bibitem_form->bibitem_form
	   && bibitem_form->bibitem_form->visible
	   && bibitem_form->bibitem_form->u_vdata == &holder)
		fl_hide_form(bibitem_form->bibitem_form);
}


void InsetBibKey::callback( FD_bibitem_form * form, long data )
{
	switch (data) {
	case 1:
		if(!holder.view->buffer()->isReadonly()) {
			setContents(fl_get_input(form->key));
			setOptions(fl_get_input(form->label));
			// shouldn't mark the buffer dirty unless
			// something was actually altered
			holder.view->updateInset( this, true );
		} // fall through to Cancel
	case 0:
		fl_hide_form(form->bibitem_form);
		break;
        }
}


void InsetBibKey::setCounter(int c) 
{ 
	counter = c; 
    
	if (getCmdName().empty())
		setCmdName( tostr(counter) );
}


// I'm sorry but this is still necessary because \bibitem is used also
// as a LyX 2.x command, and lyxlex is not enough smart to understand
// real LaTeX commands. Yes, that could be fixed, but would be a waste 
// of time cause LyX3 won't use lyxlex anyway.  (ale)
void InsetBibKey::Write(Buffer const *, ostream & os) const
{
	os << "\\bibitem ";
	if (! getOptions().empty()) {
		os << '['
		   << getOptions() << ']';
	}
	os << '{'
	   << getContents() << "}\n";
}


// This is necessary here because this is written without begin_inset
// This should be changed!!! (Jug)
void InsetBibKey::Read(Buffer const *, LyXLex & lex)
{    
	string token;

	if (lex.EatLine()) {
		token = lex.GetString();
		scanCommand(token);
	} else
		lex.printError("InsetCommand: Parse error: `$$Token'");
}


string InsetBibKey::getScreenLabel() const
{
	if (! getOptions().empty())
		return getOptions();
    
	return tostr(counter);
}


/**
  The value in "Key:" isn't allways set right after a few bibkey insets have
  been added/removed.  Perhaps the wrong object is deleted/used somewhere
  upwards?
  (Joacim 1998-03-04)
*/
void InsetBibKey::Edit(BufferView * bv, int, int, unsigned int)
{
	if(bv->buffer()->isReadonly())
		WarnReadonly(bv->buffer()->fileName());
	
	if (!bibitem_form) {
		bibitem_form = create_form_bibitem_form();
		fl_set_form_atclose(bibitem_form->bibitem_form, 
				    CancelCloseBoxCB, 0);
	}

	holder.inset = this;
	holder.view = bv;
	
	bibitem_form->bibitem_form->u_vdata = &holder;
	// InsetBibtex uses the same form, with different labels
	fl_set_object_label(bibitem_form->key, idex(_("Key:|#K")));
	fl_set_button_shortcut(bibitem_form->key, scex(_("Key:|#K")), 1);
	fl_set_object_label(bibitem_form->label, idex(_("Label:|#L")));
	fl_set_button_shortcut(bibitem_form->label, scex(_("Label:|#L")), 1);
	fl_set_input(bibitem_form->key, getContents().c_str());
	fl_set_input(bibitem_form->label, getOptions().c_str());
	if (bibitem_form->bibitem_form->visible) {
		fl_raise_form(bibitem_form->bibitem_form);
	} else {
		fl_show_form(bibitem_form->bibitem_form, FL_PLACE_MOUSE,
			     FL_FULLBORDER,
			     _("Bibliography item"));
	}   
}


InsetBibtex::InsetBibtex(string const & dbase, string const & style,
			 Buffer * o)
	: InsetCommand("BibTeX", dbase, style), owner(o)
{}


InsetBibtex::~InsetBibtex()
{
	if(bibitem_form && bibitem_form->bibitem_form
	   && bibitem_form->bibitem_form->visible
	   && bibitem_form->bibitem_form->u_vdata == &holder)
		fl_hide_form(bibitem_form->bibitem_form);
}


string InsetBibtex::getScreenLabel() const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::Latex(Buffer const *, ostream & os,
		       bool /*fragile*/, bool/*fs*/) const
{
	// this looks like an horrible hack and it is :) The problem
	// is that owner is not initialized correctly when the bib
	// inset is cut and pasted. Such hacks will not be needed
	// later (JMarc)
	if (!owner) {
		owner = current_view->buffer();
	}
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string adb, db_out;
	string db_in = getContents();
	db_in = split(db_in, adb, ',');
	while(!adb.empty()) {
		if (!owner->niceFile &&
		    IsFileReadable(MakeAbsPath(adb, owner->filepath)+".bib")) 
                         adb = MakeAbsPath(adb, owner->filepath);

		db_out += adb;
		db_out += ',';
		db_in= split(db_in, adb,',');
	}
	db_out = strip(db_out, ',');
	// Idem, but simpler
	string style;
	if (!owner->niceFile 
	    && IsFileReadable(MakeAbsPath(getOptions(), owner->filepath)
			      + ".bst")) 
		style = MakeAbsPath(getOptions(), owner->filepath);
	else
		style = getOptions();

	os << "\\bibliographystyle{" << style << "}\n"
	   << "\\bibliography{" << db_out << "}\n";
	return 2;
}


// This method returns a comma separated list of Bibtex entries
vector<pair<string,string> > InsetBibtex::getKeys() const
{
	// This hack is copied from InsetBibtex::Latex.
	// Is it still needed? Probably yes.
	// Why is this needed here when it already is in Latex?
	// Anyway we need a different way to get to the
	// buffer the inset is in. (Lgb)
	
	//if (!owner) {
	//	owner = current_view->buffer();
	//}
	
	Path p(owner->filepath);

	vector<pair<string,string> > keys;
	string tmp;
	string bibfiles = getContents();
	bibfiles = split(bibfiles, tmp, ',');
	while(!tmp.empty()) {
		string fil = findtexfile(ChangeExtension(tmp, "bib"),
					 "bib");
		lyxerr[Debug::LATEX] << "Bibfile: " << fil << endl;
		// If we didn't find a matching file name just fail silently
		if (!fil.empty()) {
			// This is a _very_ simple parser for Bibtex database
			// files. All it does is to look for lines starting
			// in @ and not being @preamble and @string entries.
			// It does NOT do any syntax checking!
			ifstream ifs(fil.c_str());
			string linebuf0;
			while (getline(ifs, linebuf0)) {
				string linebuf = frontStrip(strip(linebuf0));
				if( linebuf.empty() ) continue;
				if (prefixIs(linebuf, "@")) {
					linebuf = subst(linebuf, '{', '(');
					linebuf = split(linebuf, tmp, '(');
					tmp = lowercase(tmp);
					if (!prefixIs(tmp, "@string")
					    && !prefixIs(tmp, "@preamble")) {
						linebuf = split(linebuf, tmp, ',');
						tmp = frontStrip(tmp);
						if (!tmp.empty()) {
							keys.push_back(pair<string,string>(tmp,string()));
						}
					}
				} else if( !keys.empty() ) {
					keys.back().second += linebuf + "\n";
				}
			}
		}
		// Get next file name
    		bibfiles = split(bibfiles, tmp, ',');
	}
  	return keys;
}


// BibTeX should have its own dialog. This is provisional.
void InsetBibtex::Edit(BufferView * bv, int, int, unsigned int)
{
	if (!bibitem_form) {
		bibitem_form = create_form_bibitem_form();
		fl_set_form_atclose(bibitem_form->bibitem_form, 
				    CancelCloseBoxCB, 0);
	}

	holder.inset = this;
	holder.view = bv;
	bibitem_form->bibitem_form->u_vdata = &holder;

	fl_set_object_label(bibitem_form->key, _("Database:"));
	fl_set_object_label(bibitem_form->label, _("Style:  "));
	fl_set_input(bibitem_form->key, getContents().c_str());
	fl_set_input(bibitem_form->label, getOptions().c_str());
	if (bibitem_form->bibitem_form->visible) {
		fl_raise_form(bibitem_form->bibitem_form);
	} else {
		fl_show_form(bibitem_form->bibitem_form,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("BibTeX"));
	}   
}


bool InsetBibtex::addDatabase(string const & db)
{
	if (!contains(getContents(), db.c_str())) {
		if (!getContents().empty()) 
			addContents(",");
		addContents(db);
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const & db)
{
	if (contains(getContents(), db.c_str())) {
		string bd = db;
		int n = tokenPos(getContents(), ',', bd);
		if (n > 0) {
			// Weird code, would someone care to explain this?(Lgb)
			string tmp(", ");
			tmp += bd;
			setContents(subst(getContents(), tmp.c_str(), ", "));
		} else if (n == 0)
			setContents(split(getContents(), bd, ','));
		else 
			return false;
	}
	return true;
}


// ale070405 This function maybe shouldn't be here. We'll fix this at 0.13.
int bibitemMaxWidth(BufferView * bv, LyXFont const & font)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph * par = current_view->buffer()->paragraph;
    
	while (par) {
		if (par->bibkey) {
			int wx = par->bibkey->width(bv, font);
			if (wx > w) w = wx;
		}
		par = par->next;
	}
	return w;
}


// ale070405
string bibitemWidest(BufferView * bv)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph * par = current_view->buffer()->paragraph;
	InsetBibKey * bkey = 0;
	LyXFont font;
      
	while (par) {
		if (par->bibkey) {
			int wx = par->bibkey->width(bv, font);
			if (wx > w) {
				w = wx;
				bkey = par->bibkey;
			}
		}
		par = par->next;
	}
    
	if (bkey && !bkey->getScreenLabel().empty())
		return bkey->getScreenLabel();
    
	return "99";
}
