/*
 *  File:        formula.h
 *  Purpose:     Implementation of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: (c) 1996-1998 Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include <cctype>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation "formula.h"
#endif

#include "formula.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "lyx_main.h"
#include "bufferlist.h"
#include "lyx_cb.h"
#include "minibuffer.h"
#include "BufferView.h"
#include "lyxscreen.h"
#include "lyxdraw.h"
#include "lyxtext.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "lyx_gui_misc.h"

extern void UpdateInset(Inset* inset, bool mark_dirty = true);
extern void LockedInsetStoreUndo(Undo::undo_kind);
extern MiniBuffer *minibuffer;
extern void ShowLockedInsetCursor(long, long, int, int);
extern void HideLockedInsetCursor(long, long, int, int);
extern void FitLockedInsetCursor(long, long, int, int);
extern int LockInset(UpdatableInset*);
extern int UnlockInset(UpdatableInset*);


extern GC canvasGC, mathGC, mathLineGC, latexGC, cursorGC, mathFrameGC;
extern char *mathed_label;

extern int mono_video;
extern int fast_selection;

extern BufferView *current_view;
extern BufferList bufferlist;
extern char const *latex_special_chars;

short greek_kb_flag = 0;

LyXFont *Math_Fonts = 0; // this is only used by Whichfont and mathed_init_fonts (Lgb)

static LyXFont::FONT_SIZE lfont_size = LyXFont::SIZE_NORMAL;

// local global 
static int sel_x, sel_y;
static bool sel_flag;
MathedCursor* InsetFormula::mathcursor = 0; 


int MathedInset::df_asc;
int MathedInset::df_des;
int MathedInset::df_width;

// wrong name on this one should be called "IsAscii"
inline bool IsAlpha(char c)
{
   return ('A' <= c  && c<= 'Z' || 'a' <= c  && c<= 'z');
}

inline bool IsDigit(char c)
{
   return ('0' <= c && c <= '9');
}

inline bool IsMacro(short token, int id)
{
   return (token!= LM_TK_FRAC && token!= LM_TK_SQRT &&
	  !((token == LM_TK_SYM || token == LM_TC_BSYM) && id<255));
}

void mathedValidate(LaTeXFeatures &features, MathParInset *par);

LyXFont WhichFont(short type, int size)
{
    LyXFont f;
    
      if (!Math_Fonts)
	mathed_init_fonts();
   
   switch (type) {
    case LM_TC_SYMB:	     
      f = Math_Fonts[2];
      break;
    case LM_TC_BSYM:	     
      f = Math_Fonts[2];
      break;
    case LM_TC_VAR:
    case LM_TC_IT:
      f = Math_Fonts[0];
      break;
    case LM_TC_BF:
      f = Math_Fonts[3];
      break;
    case LM_TC_SF:
      f = Math_Fonts[7];
      break;
    case LM_TC_CAL:
      f = Math_Fonts[4];
      break;
    case LM_TC_TT:
      f = Math_Fonts[5];
      break;
    case LM_TC_SPECIAL: //f = Math_Fonts[0]; break;
    case LM_TC_TEXTRM:
    case LM_TC_RM:    
      f = Math_Fonts[6];
      break;
    default:
      f = Math_Fonts[1];
      break;   
   }
    
    f.setSize(lfont_size);
    
    switch (size) {
     case LM_ST_DISPLAY:     
	if (type == LM_TC_BSYM) {
	    f.incSize();
	    f.incSize();
	}
	break;
     case LM_ST_TEXT:
	break;
     case LM_ST_SCRIPT:
	f.decSize();
	break;
     case LM_ST_SCRIPTSCRIPT:
	f.decSize();
	f.decSize();
	break;
     default:
	     lyxerr << "Mathed Error: wrong font size: " << size << endl;
	break;
    }
    
    if (type!= LM_TC_TEXTRM) 
      f.setColor(LyXFont::MATH);
    return f;
}


void mathed_init_fonts() //removed 'static' because DEC cxx does not
			 //like it (JMarc) 
{

    Math_Fonts = new LyXFont[8]; //DEC cxx cannot initialize all fonts
				 //at once (JMarc) rc
    for (int i= 0 ; i<8 ; i++){ 
    	Math_Fonts[i] = LyXFont::ALL_SANE;
    }
    Math_Fonts[0].setShape(LyXFont::ITALIC_SHAPE);
    
    Math_Fonts[1].setFamily(LyXFont::SYMBOL_FAMILY);
    
    Math_Fonts[2].setFamily(LyXFont::SYMBOL_FAMILY);
    Math_Fonts[2].setShape(LyXFont::ITALIC_SHAPE);

    Math_Fonts[3].setSeries(LyXFont::BOLD_SERIES);
      
    Math_Fonts[4].setFamily(LyXFont::SANS_FAMILY);
    Math_Fonts[4].setShape(LyXFont::ITALIC_SHAPE);
      
    Math_Fonts[5].setFamily(LyXFont::TYPEWRITER_FAMILY);

    Math_Fonts[6].setFamily(LyXFont::ROMAN_FAMILY);

    Math_Fonts[7].setFamily(LyXFont::SANS_FAMILY);
    
    LyXFont f = WhichFont(LM_TC_VAR, LM_ST_TEXT);
    MathedInset::df_asc = f.maxAscent(); 
    MathedInset::df_des = f.maxDescent();
    MathedInset::df_width = f.width('I');    
}


void mathed_set_font(short type, int size)
{
    if (!canvasGC) {
	    cursorGC = getGC(gc_thin_on_off_line);
	    canvasGC = getGC(gc_lighted);
	    latexGC =  getGC(gc_latex);
	    mathLineGC = getGC(gc_math);
	    mathFrameGC = getGC(gc_math_frame);
    }
    LyXFont f = WhichFont(type, size); 
    if (type == LM_TC_TEX) {
	f.setLatex(LyXFont::ON);
	latexGC = f.getGC();
    } else
      mathGC = f.getGC();
}


int mathed_string_width(short type, int size, byte const* s, int ls)
{
    LyXFont f = WhichFont(type, size);

    byte sx[80];
    if (MathIsBinary(type)) {
	byte *ps = &sx[0];
	for (int i= 0; i<ls && i<75; i++) {
	    *(ps++) = ' ';
	    *(ps++) = s[i];
	    *(ps++) = ' ';
	}
	*(ps++) = '\0';
	ls = 3*ls;
	s = &sx[0];
    }
    return f.textWidth((const char*)s, ls);;
}

int mathed_char_width(short type, int size, byte c)
{
    int t= (MathIsBinary(type)) ? mathed_string_width(type, size, &c, 1):
           WhichFont(type, size).width(c);
    return t;
}

int mathed_string_height(short type, int size, byte const * s, int ls, int& asc, int& des)
{
   LyXFont font = WhichFont(type, size);
   asc = des = 0;
   for (int i= 0; i<ls; i++) {
      if (font.descent(s[i]) > des)
	des = font.descent(s[i]);
      if (font.ascent(s[i]) > asc)
	asc = font.ascent(s[i]);
   }
   return asc+des;
}

int mathed_char_height(short type, int size, byte c, int& asc, int& des)
{
   LyXFont font = WhichFont(type, size);
   asc = des = 0;
   des = font.descent(c);
   asc = font.ascent(c);
   return asc+des;
}


// In a near future maybe we use a better fonts renderer
void MathedInset::drawStr(short type, int siz, int x, int y, byte* s, int ls)
{
    mathed_set_font(type, siz);
    byte sx[80];
    if (MathIsBinary(type)) {
	byte *ps = &sx[0];
	for (int i= 0; i<ls && i < 75; i++) {
	    *(ps++) = ' ';
	    *(ps++) = s[i];
	    *(ps++) = ' ';
	}
	//    *ps = ' ';
	ls = 3*ls;
	s = &sx[0];
    }
    GC gc = (type == LM_TC_TEX) ? latexGC: mathGC;
    XDrawString(fl_display, pm, gc, x, y, reinterpret_cast<char*>(s), ls);
    XFlush(fl_display);
}


InsetFormula::InsetFormula(bool display)
{
  par = new MathParInset; // this leaks
  //   mathcursor = 0;
  disp_flag = display;
  //label = 0;
  if (disp_flag) {
    par->SetType(LM_OT_PAR);
    par->SetStyle(LM_ST_DISPLAY);
  }
}

InsetFormula::InsetFormula(MathParInset *p)
{
   par = (p->GetType()>= LM_OT_MPAR) ? 
         new MathMatrixInset((MathMatrixInset*)p): 
         new MathParInset(p);
//   mathcursor = 0;
   
   disp_flag = (par->GetType()>0);
   //label = 0;
}

InsetFormula::~InsetFormula()
{
   delete par;
   //if (label) delete label;
}

InsetFormula * InsetFormula::Clone() const
{
    InsetFormula * f = new InsetFormula(par);
    f->label = label;
    return f;
}

void InsetFormula::Write(FILE *file)
{
   fprintf(file, "Formula ");
   Latex(file, 0);
}

int InsetFormula::Latex(FILE *file, signed char fragile)
{
    int ret = 0;      
//#warning Alejandro, the number of lines is not returned in this case
// This problem will disapear at 0.13.
    string output;
    InsetFormula::Latex(output, fragile);
    fprintf(file, "%s", output.c_str());
    return ret;
}

int InsetFormula::Latex(string &file, signed char fragile)
{
    int ret = 0;        
//#warning Alejandro, the number of lines is not returned in this case
// This problem will disapear at 0.13.
    if (fragile < 0)
        par->Write(file);
    else
        mathed_write(par, file, &ret, fragile, label.c_str());
    return ret;
}


int InsetFormula::Linuxdoc(string &/*file*/)
{
    return 0;
}


int InsetFormula::DocBook(string &/*file*/)
{
    return 0;
}


// Check if uses AMS macros 
void InsetFormula::Validate(LaTeXFeatures &features) const
{
    // Validation only necesary if not using an AMS Style
    if (!features.amsstyle)
      mathedValidate(features, par);
}


void InsetFormula::Read(LyXLex &lex)
{
    FILE *file = lex.getFile();
    
    mathed_parser_file(file, lex.GetLineNo());   
   
   // Silly hack to read labels. 
   mathed_label = 0;
   mathed_parse(0, 0, &par);
   par->Metrics();
   disp_flag = (par->GetType()>0);
   
    // Update line number
    lex.setLineNo(mathed_parser_lineno());
    
   if (mathed_label) {
      label = mathed_label;
      mathed_label = 0;
   }
   
#ifdef DEBUG
   Write(stdout);
   fflush(stdout);
#endif
}

int InsetFormula::Ascent(LyXFont const &) const
{
   return par->Ascent() + ((disp_flag) ? 8: 1);
}

int InsetFormula::Descent(LyXFont const &) const
{
   return par->Descent() + ((disp_flag) ? 8: 1);
}

int InsetFormula::Width(LyXFont const &f) const
{
    lfont_size = f.size();
    par->Metrics();
    return par->Width(); //+2;
}

void InsetFormula::Draw(LyXFont f, LyXScreen &scr, int baseline, float &x)
{
	// This is Alejandros domain so I'll use this
	unsigned long pm = scr.getForeground();
	
   lfont_size = f.size();
   mathed_set_font(LM_TC_TEXTRM, LM_ST_TEXT); // otherwise a segfault could occur
								// in some XDrawRectangles (i.e. matrix) (Matthias)   
   if (mathcursor && mathcursor->GetPar() == par) { 
       if (mathcursor->Selection()) {
	   int n;
	   XPoint * p = mathcursor->SelGetArea(n);
	   XFillPolygon(fl_display, pm, getGC(gc_selection), //LyXGetSelectionGC(),
			p, n, Nonconvex, CoordModeOrigin);
       }
       mathcursor->Draw(pm, (int)x, baseline);
   } else {
//       par->Metrics();
       par->setDrawable(pm);
       par->Draw((int)x, baseline);
   }
   x += (float)Width(f);
 
   if (par->GetType() == LM_OT_PARN || par->GetType() == LM_OT_MPARN) {
       char s[80];
       LyXFont  font = WhichFont(LM_TC_BF, par->size);
       font.setLatex(LyXFont::OFF);
      
       if (par->GetType() == LM_OT_PARN) {
	   if (!label.empty())
	     sprintf(s, "(%s)", label.c_str());
	   else
	     sprintf(s, "(#)");
	   font.drawString(s, pm, baseline, int(x+20));
       } else 
       if (par->GetType() == LM_OT_MPARN) {
	   MathMatrixInset *mt = (MathMatrixInset*)par;
	   //int i= 0;
	   int y;
	   MathedRowSt const* crow = mt->getRowSt();
	   while (crow) {
	       y = baseline + crow->getBaseline();
	       if (crow->isNumbered()) {
		   if (crow->getLabel())
		     sprintf(s, "(%s)", crow->getLabel());
		   else
		     sprintf(s, "(#)");
		   font.drawString(s, pm, y, int(x+20));
	       }
	       crow = crow->getNext();
	   }
       }
   }
   cursor_visible = false;
}


void InsetFormula::Edit(int x, int y)
{
   mathcursor = new MathedCursor(par);
   LockInset(this);
   par->Metrics();
   UpdateInset(this, false);
   x += par->xo; 
   y += par->yo; 
   mathcursor->SetPos(x, y);
    sel_x = sel_y = 0;
    sel_flag = false;
}
				       
void InsetFormula::InsetUnlock()
{
   if (mathcursor) {
       if (mathcursor->InMacroMode()) {
	   mathcursor->MacroModeClose();
	   UpdateLocal();
       }                                         
     delete mathcursor;
   }
   mathcursor = 0;
   UpdateInset(this, false);
}

// Now a symbol can be inserted only if the inset is locked
void InsetFormula::InsertSymbol(char const* s)
{ 
   if (!s || !mathcursor) return;   
   mathcursor->Interpret(s);
   UpdateLocal();
}
   
void InsetFormula::GetCursorPos(int& x, int& y) const
{
    mathcursor->GetPos(x, y);
    x -= par->xo; 
    y -= par->yo;
}

void InsetFormula::ToggleInsetCursor()
{
  if (!mathcursor)
    return;

  int x, y, asc, desc;
  mathcursor->GetPos(x, y);
//  x -= par->xo; 
  y -= par->yo; 
    LyXFont font = WhichFont(LM_TC_TEXTRM, LM_ST_TEXT);
  asc = font.maxAscent();
  desc = font.maxDescent();
  
  if (cursor_visible)
    HideLockedInsetCursor(x, y, asc, desc);
  else
    ShowLockedInsetCursor(x, y, asc, desc);
  cursor_visible = !cursor_visible;
}

void InsetFormula::ShowInsetCursor(){
  if (!cursor_visible){
    int x, y, asc, desc;
    if (mathcursor){
      mathcursor->GetPos(x, y);
      //  x -= par->xo; 
      y -= par->yo;
	LyXFont font = WhichFont(LM_TC_TEXTRM, LM_ST_TEXT);
	asc = font.maxAscent();
	desc = font.maxDescent();
      FitLockedInsetCursor(x, y, asc, desc);
    }
    ToggleInsetCursor();
  }
}

void InsetFormula::HideInsetCursor(){
  if (cursor_visible)
    ToggleInsetCursor();
}

void InsetFormula::ToggleInsetSelection()
{
    if (!mathcursor)
      return;
    
//    int x, y, w, h;
    //int n;
    //XPoint * p = 
    //mathcursor->SelGetArea(n);
//    XFillPolygon(fl_display, pm, LyXGetSelectionGC(), p, n, Nonconvex, CoordModeOrigin);
//    x -= par->xo; 
//    y -= par->yo;

    UpdateInset(this, false);
      
}

void InsetFormula::display(bool dspf)
{
   if (dspf!= disp_flag) {
      if (dspf) {
	 par->SetType(LM_OT_PAR);
	 par->SetStyle(LM_ST_DISPLAY);
      } else {
	 if (par->GetType()>= LM_OT_MPAR) { 
	    MathParInset * p = new MathParInset(par);
	    delete par;
	    par = p;
	    if (mathcursor) 
	       mathcursor->SetPar(par); 
	 }
	 par->SetType(LM_OT_MIN);
	 par->SetStyle(LM_ST_TEXT);
	 if (!label.empty() && par->GetType()!= LM_OT_MPARN) {
		 label.clear();
	 }
      }
      disp_flag = dspf;
   }   
}
   


int InsetFormula::GetNumberOfLabels() const
{
   // This is dirty, I know. I'll clean it at 0.13
   if (par->GetType() == LM_OT_MPARN) {
       MathMatrixInset * mt = (MathMatrixInset*)par;
       int nl = 0;
       MathedRowSt const * crow = mt->getRowSt();
       while (crow) {
	   if (crow->getLabel()) nl++;
	   crow = crow->getNext();
       }
       return nl;
   } else
   if (!label.empty())
       return 1;
   else
       return 0;
}


string InsetFormula::getLabel(int il) const
{
//#warning This is dirty, I know. Ill clean it at 0.11
	// Correction, the only way to clean this is with a new kernel: 0.13.
	if (par->GetType() == LM_OT_MPARN) {
		string lab;
		MathMatrixInset * mt = (MathMatrixInset*)par;
		int nl= 0;
		MathedRowSt const * crow = mt->getRowSt();
		while (crow) {
			if (crow->getLabel()) {
				if (nl == il) {
					lab = crow->getLabel();
					break;
				}
				nl++;
			}
			crow = crow->getNext();
		}
		return lab;
	}
	return label;
}

void InsetFormula::UpdateLocal()
{
   par->Metrics();  // To inform lyx kernel the exact size 
                  // (there were problems with arrays).
   UpdateInset(this);
}
   


void InsetFormula::InsetButtonRelease(int x, int y, int /*button*/)
{
    HideInsetCursor();
    x += par->xo;
    y += par->yo;
    mathcursor->SetPos(x, y);
    ShowInsetCursor();
    if (sel_flag) {
	sel_flag = false; 
	sel_x = sel_y = 0;
	UpdateInset(this, false); 
    }
}

void InsetFormula::InsetButtonPress(int x, int y, int /*button*/)
{
    sel_flag = false;
    sel_x = x;  sel_y = y; 
    if (mathcursor->Selection()) {
	mathcursor->SelClear();
	UpdateInset(this, false); 
    }
}

void InsetFormula::InsetMotionNotify(int x, int y, int /*button*/)
{
    if (sel_x && sel_y && abs(x-sel_x)>4 && !sel_flag) {
	sel_flag = true;
	HideInsetCursor();
	mathcursor->SetPos(sel_x + par->xo, sel_y + par->yo);
	mathcursor->SelStart();
	ShowInsetCursor(); 
	mathcursor->GetPos(sel_x, sel_y);
    } else
      if (sel_flag) {
	  HideInsetCursor();
	  x += par->xo;
	  y += par->yo;
	  mathcursor->SetPos(x, y);
	  ShowInsetCursor();
	  mathcursor->GetPos(x, y);
	  if (sel_x!= x || sel_y!= y)
	    UpdateInset(this, false); 
	  sel_x = x;  sel_y = y;
      }
}

void InsetFormula::InsetKeyPress(XKeyEvent *)
{
	lyxerr[Debug::MATHED] << "Used InsetFormula::InsetKeyPress." << endl;
}

// Special Mathed functions
bool InsetFormula::SetNumber(bool numbf)
{
   if (disp_flag) {
      short type = par->GetType();
      bool oldf = (type == LM_OT_PARN || type == LM_OT_MPARN);
      if (numbf && !oldf) type++;
      if (!numbf && oldf) type--;
      par->SetType(type);
      return oldf;
   } else
     return false;
}

bool InsetFormula::LocalDispatch(int action, char const *arg)
{
//   extern char *dispatch_result;
    MathedTextCodes varcode = LM_TC_MIN;       
   bool was_macro = mathcursor->InMacroMode();
   bool sel= false;
   bool space_on = false;
   bool was_selection = mathcursor->Selection();
   bool result = true;
   static MathSpaceInset* sp= 0;

   HideInsetCursor();
   if (mathcursor->Selection() && (fast_selection || mono_video)) ToggleInsetSelection();

    if (mathcursor->getLastCode() == LM_TC_TEX) { 
	varcode = LM_TC_TEX;
    }
   switch (action) {
       
    // --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL: sel = true;
    case LFUN_RIGHT:
      {
	 result = mathcursor->Right(sel);
	 break;
      }
    case LFUN_LEFTSEL: sel = true;     
    case LFUN_LEFT:
      {
	 result = mathcursor->Left(sel);
	 break;
      }
    case LFUN_UPSEL: sel = true;  
    case LFUN_UP:
      result = mathcursor->Up(sel);
      break;
       
    case LFUN_DOWNSEL: sel = true;  
    case LFUN_DOWN:
      result = mathcursor->Down(sel);
      break;
    case LFUN_HOME:
      mathcursor->Home();
      break;
    case LFUN_END:
      mathcursor->End();
      break;
    case LFUN_DELETE_LINE_FORWARD:
	    //LockedInsetStoreUndo(Undo::INSERT);
	    LockedInsetStoreUndo(Undo::DELETE);
      mathcursor->DelLine();
      UpdateLocal();
      break;
    case LFUN_BREAKLINE:
      LockedInsetStoreUndo(Undo::INSERT);
      mathcursor->Insert(' ', LM_TC_CR);
      par = mathcursor->GetPar();
      UpdateLocal();     
      break;
    case LFUN_TAB:
      LockedInsetStoreUndo(Undo::INSERT);
      mathcursor->Insert(0, LM_TC_TAB);
      //UpdateInset(this);
      break;     
    case LFUN_TABINSERT:
      LockedInsetStoreUndo(Undo::INSERT);
      mathcursor->Insert('T', LM_TC_TAB);
      UpdateLocal();
      break;     
    case LFUN_BACKSPACE:
       if (!mathcursor->Left()) 
	 break;
       
       if (!mathcursor-> InMacroMode() && mathcursor->pullArg()) {       
	   UpdateInset(this);
	   break;
       }
      
    case LFUN_DELETE:
	    //LockedInsetStoreUndo(Undo::INSERT);
	    LockedInsetStoreUndo(Undo::DELETE);
      mathcursor->Delete();       
      UpdateInset(this);
      break;    
//    case LFUN_GETXY:
//      sprintf(dispatch_buffer, "%d %d",);
//      dispatch_result = dispatch_buffer;
//      break;
    case LFUN_SETXY:
      {
	 int x, y, x1, y1;
         sscanf(arg, "%d %d", &x, &y);
	 par->GetXY(x1, y1);
	 mathcursor->SetPos(x1+x, y1+y);
      }
      break;

      /* cursor selection ---------------------------- */

    case LFUN_PASTE:
            if (was_macro)
		mathcursor->MacroModeClose();
	    LockedInsetStoreUndo(Undo::INSERT);
	    mathcursor->SelPaste(); UpdateLocal(); break;
    case LFUN_CUT:
	    LockedInsetStoreUndo(Undo::DELETE);
	    mathcursor->SelCut(); UpdateLocal(); break;
    case LFUN_COPY: mathcursor->SelCopy(); break;      
    case LFUN_HOMESEL:
    case LFUN_ENDSEL:
    case LFUN_WORDRIGHTSEL:
    case LFUN_WORDLEFTSEL:
      break;
      
    // --- accented characters ------------------------------

    case LFUN_UMLAUT: mathcursor->setAccent(LM_ddot); break;
    case LFUN_CIRCUMFLEX: mathcursor->setAccent(LM_hat); break;
    case LFUN_GRAVE: mathcursor->setAccent(LM_grave); break;
    case LFUN_ACUTE: mathcursor->setAccent(LM_acute); break;
    case LFUN_TILDE: mathcursor->setAccent(LM_tilde); break;
    case LFUN_MACRON: mathcursor->setAccent(LM_bar); break;
    case LFUN_DOT: mathcursor->setAccent(LM_dot); break;
    case LFUN_CARON: mathcursor->setAccent(LM_check); break;
    case LFUN_BREVE: mathcursor->setAccent(LM_breve); break;
    case LFUN_VECTOR: mathcursor->setAccent(LM_vec); break; 
      
    // Greek mode     
    case LFUN_GREEK:
    {
       if (!greek_kb_flag) {
	  greek_kb_flag = 1;
	  minibuffer->Set(_("Math greek mode on"));
       } else
	 greek_kb_flag = 0;
       break;
    }  
      
    // Greek keyboard      
    case LFUN_GREEK_TOGGLE:
    {
       greek_kb_flag = (greek_kb_flag) ? 0: 2;
       if (greek_kb_flag)
	 minibuffer->Set(_("Math greek keyboard on"));
       else
	 minibuffer->Set(_("Math greek keyboard off"));
       break;
    }  
   
      //  Math fonts 
    case LFUN_BOLD:  	mathcursor->setLastCode(LM_TC_BF); break;
    case LFUN_SANS:  mathcursor->setLastCode( LM_TC_SF); break;
    case LFUN_EMPH:  mathcursor->setLastCode(LM_TC_CAL); break;
    case LFUN_ROMAN: mathcursor->setLastCode(LM_TC_RM); break;
    case LFUN_CODE: mathcursor->setLastCode(LM_TC_TT); break;   
    case LFUN_DEFAULT:  mathcursor->setLastCode(LM_TC_VAR ) ; break;
    case LFUN_TEX: 
    {
//       varcode = LM_TC_TEX;
	mathcursor->setLastCode(LM_TC_TEX);
       minibuffer->Set(_("TeX mode")); 
       break;
    }

    case LFUN_MATH_NUMBER:
    {
      LockedInsetStoreUndo(Undo::INSERT);
       if (disp_flag) {
	  short type = par->GetType();
	  bool oldf = (type == LM_OT_PARN || type == LM_OT_MPARN);
	  if (oldf) {
	     type--;
	     if (!label.empty()) {
		     label.clear();
	     }
	     minibuffer->Set(_("No number"));  
	  } else {
	     type++;
             minibuffer->Set(_("Number"));
	  }
	  par->SetType(type);
	  UpdateLocal();
       }
       break;
    }
    
    case LFUN_MATH_NONUMBER:
    { 
	if (par->GetType() == LM_OT_MPARN) {
//	   MathMatrixInset *mt = (MathMatrixInset*)par;
	   //BUG 
//	   mt->SetNumbered(!mt->IsNumbered());
	    
	    mathcursor->setNumbered();
	   UpdateLocal();
	}
	break;
    }
       
    case LFUN_MATH_LIMITS:
    {
      LockedInsetStoreUndo(Undo::INSERT);
       if (mathcursor->Limits())
	 UpdateLocal();
    }
 
    case LFUN_MATH_SIZE:
       if (arg) {
	   latexkeys *l = in_word_set (arg, strlen(arg));
	   int sz = (l) ? l->id: -1;
	   mathcursor->SetSize(sz);
	   UpdateLocal();
	   break;
       }
       
    case LFUN_INSERT_MATH:
    {
	LockedInsetStoreUndo(Undo::INSERT);
	InsertSymbol(arg);
	break;
    }
    
    case LFUN_INSERT_MATRIX:
    { 
      LockedInsetStoreUndo(Undo::INSERT);
       int k, m, n;
       char s[80], arg2[80];
       // This is just so that too long args won't ooze out of s.
       strncpy(arg2, arg, 80); arg2[79]= (char)0;
       k = sscanf(arg2, "%d %d %s", &m, &n, s);
       s[79] = (char)0;
	
       if (k<1) {
	   m = n = 1;
       } else if (k == 1) {
	   n = 1;
       }
	
       MathMatrixInset *p = new MathMatrixInset(m, n);      
       if (mathcursor && p) {
	  if (k>2 && (int)strlen(s)>m)
	    p->SetAlign(s[0], &s[1]);
	  mathcursor->Insert(p, LM_TC_ACTIVE_INSET);
	  UpdateLocal();     
       }
       break;
    }
      
    case LFUN_MATH_DELIM:
    {  
      LockedInsetStoreUndo(Undo::INSERT);
       char lf[40], rg[40], arg2[40];
       int ilf = '(', irg = '.';
       latexkeys *l;
       string vdelim("(){}[]./|");
	
       if (!arg) break;
       strncpy(arg2, arg, 40); arg2[39]= (char)0;
       int n = sscanf(arg2, "%s %s", lf, rg);
       lf[39] = (char)0; rg[39] = (char)0;

       if (n>0) {
	   if (IsDigit(lf[0])) 
	     ilf = atoi(lf);
	   else 
	     if (lf[1]) {
		 l = in_word_set(lf, strlen(lf));
		 // Long words will cause l == 0; so check.
		 if(l) ilf = l->id;
	     } else
	     if (vdelim.find(lf[0]) != string::npos)
	       ilf = lf[0];
	   
	   if (n>1) {
	       if (IsDigit(rg[0]))
		 irg = atoi(rg);
	       else 
		 if (rg[1]) {
		     l = in_word_set(rg, strlen(rg));
		     if(l) irg = l->id;
		 } else
		 if (vdelim.find(rg[0]) != string::npos)
		   irg = rg[0];
	   }
       }
       
       MathDelimInset* p = new MathDelimInset(ilf, irg);
       mathcursor->Insert(p, LM_TC_ACTIVE_INSET);
       UpdateLocal();                             
       break;
    }

    case LFUN_PROTECTEDSPACE:
    {
      LockedInsetStoreUndo(Undo::INSERT);
       sp = new MathSpaceInset(1); 
       mathcursor->Insert(sp);
       space_on = true;
       UpdateLocal();
       break;
    }
      
    case LFUN_INSERT_LABEL:
    {
      LockedInsetStoreUndo(Undo::INSERT);
       if (par->GetType()<LM_OT_PAR) break;
       string lb = arg;
       if (lb.empty())
	      lb = string(askForText(_("Enter new label to insert:")));
       if (!lb.empty() && lb[0]> ' ') {
	  SetNumber(true);
	  if (par->GetType() == LM_OT_MPARN) {
	      mathcursor->setLabel(lb.c_str());
//	      MathMatrixInset *mt = (MathMatrixInset*)par;
//	      mt->SetLabel(lb);
	  } else {
		  //if (label.notEmpty()) delete label;
	      label = lb;
	  }
	  UpdateLocal();
       } else
	       label.clear();
	       //label = 0;
       break;
    }
    
    case LFUN_MATH_DISPLAY:
	    //LockedInsetStoreUndo(Undo::INSERT);
	    LockedInsetStoreUndo(Undo::EDIT);
      display(!disp_flag);
      UpdateLocal();
      break;
      
    // Invalid actions under math mode
    case LFUN_MATH_MODE:  
    {
	if (mathcursor->getLastCode()!= LM_TC_TEXTRM) {
	    minibuffer->Set(_("math text mode"));
	    varcode = LM_TC_TEXTRM;
	} else {
	    varcode = LM_TC_VAR;
	}
	mathcursor->setLastCode(varcode);
	break; 
    }
    case LFUN_UNDO:
      minibuffer->Set(_("Invalid action in math mode!"));
      break;

    //------- dummy actions
    case LFUN_EXEC_COMMAND:
       minibuffer->ExecCommand(); 
       break;
       
    default:
      if ((action == -1  || action == LFUN_SELFINSERT) && arg)  {
	 unsigned char c = arg[0];
	 LockedInsetStoreUndo(Undo::INSERT);
	 
	 if (c == ' ' && mathcursor->getAccent() == LM_hat) {
	     c = '^';
	     mathcursor->setAccent(0);
	 }
	 if (c == 0) {      // Dead key, do nothing 
	     //lyxerr << "deadkey" << endl;
	     break;
	 } 
	 if (isalpha(c)) {
	     if (mathcursor->getLastCode() == LM_TC_TEX) { 
	       mathcursor->MacroModeOpen();
	       mathcursor->clearLastCode();
	       varcode = LM_TC_MIN;
	    } else	    
	    if (!varcode) {		
		short f = (mathcursor->getLastCode()) ? 
		          mathcursor->getLastCode():
		          (MathedTextCodes)mathcursor->GetFCode();
	       varcode =  MathIsAlphaFont(f) ?  (MathedTextCodes)f:LM_TC_VAR;
	    }

//	     lyxerr << "Varcode << vardoce;
	    mathcursor->Insert(c, (greek_kb_flag) ? LM_TC_SYMB: varcode);
	    varcode = LM_TC_MIN;
	    if (greek_kb_flag<2) greek_kb_flag = 0;
	 } else 
	   if (strchr("!,:;{}", c) && (varcode == LM_TC_TEX||was_macro)) {
	       mathcursor->Insert(c, LM_TC_TEX);
	       if (c == '{') {
		   mathcursor->Insert('}', LM_TC_TEX);
		   mathcursor->Left();
	       }
	       mathcursor->clearLastCode();
//	       varcode = LM_TC_MIN;
	   } else
	   if (c == '_' && varcode == LM_TC_TEX) {
	       mathcursor->Insert(c, LM_TC_SPECIAL);
	       mathcursor->clearLastCode();
//	       varcode = LM_TC_MIN;
	   } else
	    if (('0'<= c && c<= '9') && (varcode == LM_TC_TEX||was_macro)) {
		mathcursor->MacroModeOpen();
		mathcursor->clearLastCode();
		mathcursor->Insert(c, LM_TC_MIN);
	    }
	 else
	   if (('0'<= c && c<= '9') || strchr(";:!|[]().,?", c)) 
	      mathcursor->Insert(c, LM_TC_CONST);
	 else
	   if (strchr("+/-*<>= ", c))
	      mathcursor->Insert(c, LM_TC_BOP);
	 else
	   if (strchr(latex_special_chars, c) && c!= '_')
	      mathcursor->Insert(c, LM_TC_SPECIAL);
	 else
	   if (c == '_' || c == '^') {
	       char s[2];
	       s[0] = c;
	       s[1] = 0;
	      mathcursor->Interpret (s);
	   } else
	   if (c == ' ') {	    
	       if (!varcode) {	
		   short f = (mathcursor->getLastCode()) ? 
		              mathcursor->getLastCode():
		              (MathedTextCodes)mathcursor->GetFCode();
		   varcode = MathIsAlphaFont(f) ? (MathedTextCodes)f:LM_TC_VAR;
	       }
	      if (varcode == LM_TC_TEXTRM) {
		  mathcursor->Insert(c, LM_TC_TEXTRM);
	      } else
	      if (was_macro)
		mathcursor->MacroModeClose();
	      else 
	      if (sp) {
		 int isp = (sp->GetSpace()<5) ? sp->GetSpace()+1: 0;
		 sp->SetSpace(isp);
		 space_on = true;
	      } else {
		  if (!mathcursor->Pop() && mathcursor->IsEnd()) 
		    result = false;
	      }
	   } else
	   if (c == '\'') {
	      mathcursor->Insert (c, LM_TC_VAR);
	   } else
	   if (c == '\\') {
	      if (was_macro)
		mathcursor->MacroModeClose();
	      minibuffer->Set(_("TeX mode")); 
	       mathcursor->setLastCode(LM_TC_TEX);
	   } 
	 UpdateLocal();
      } else {
	// lyxerr << "Closed by action " << action << endl;
	result =  false;
      }
   }
   if (was_macro!= mathcursor->InMacroMode()&&action>= 0&&action!= LFUN_BACKSPACE)
     UpdateLocal();
   if (sp && !space_on) sp = 0;
   if (mathcursor->Selection() || (was_selection && !(fast_selection || mono_video)))
       ToggleInsetSelection();
    
   if (result)
      ShowInsetCursor();
   else
      UnlockInset(this);
    
   return result;
}


void
MathFuncInset::Draw(int x, int y)
{ 
	if (name && name[0]>' ') {
		LyXFont  font = WhichFont(LM_TC_TEXTRM, size);
		font.setLatex(LyXFont::ON);
	        x += (font.textWidth("I", 1)+3)/4;
		if (mono_video) {
			int a= font.maxAscent(), d= font.maxDescent();
			XFillRectangle (fl_display, pm, getGC(gc_copy),
					x, y-a,
					font.textWidth(name, strlen(name)), a+d);
			XFlush(fl_display);
		}
		font.drawString(name, pm, y, x);
	}
}


void MathFuncInset::Metrics() 
{
	ln = (name) ? strlen(name): 0;
	LyXFont  font = WhichFont(LM_TC_TEXTRM, size);
	font.setLatex(LyXFont::ON);
	width = font.textWidth(name, ln) + font.textWidth("I", 1)/2;
	mathed_string_height(LM_TC_TEXTRM, size, (byte const *) name,
			     strlen(name), ascent, descent);
}


void mathedValidate(LaTeXFeatures &features, MathParInset *par)
{
    MathedIter it(par->GetData());
    
    while (it.OK() && !(features.binom && features.boldsymbol)) {
	if (it.IsInset()) {
	    if(it.IsActive()) {
		MathParInset *p = it.GetActiveInset();
		if (!features.binom && p->GetType() == LM_OT_MACRO && 
		    strcmp(p->GetName(), "binom") == 0) {
		    features.binom = true;
		} else {
		    for (int i= 0; i<= p->getMaxArgumentIdx(); i++) {
			p->setArgumentIdx(i);
			mathedValidate(features, p);
		    }
		}
	    } else {
		MathedInset* p = it.GetInset();
		if (!features.boldsymbol && p->GetName() &&
		    strcmp(p->GetName(), "boldsymbol") == 0) {
		    features.boldsymbol = true;
		}
	    }	    
	}
	it.Next();
    }
}
