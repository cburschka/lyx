// -*- C++ -*-
/*
 *  File:        math_macro.h
 *  Purpose:     Declaration of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Mathed
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.2, Mathed & Lyx project.
 *
 *   This code is under the GNU General Public Licence version 2 or later.
 */
#ifndef MATH_MACRO
#define MATH_MACRO

#ifdef __GNUG__
#pragma interface
#endif

#include "math_defs.h"
#include "debug.h"

///
typedef MathParInset * MathParInsetP;
///
typedef LyxArrayBase * LyxArrayBaseP;

class MathMacroTemplate;


/// This class contains the data for a macro
class MathMacro : public MathParInset
{
public:
    /// A macro can only be builded from an existing template
    MathMacro(MathMacroTemplate *);
    /// or from another macro.
    MathMacro(MathMacro *);
	///
    ~MathMacro();
    ///
    void draw(Painter &, int, int);
    ///
    void Metrics();
	///
    MathedInset * Clone();
	///
    void Write(ostream &);
#ifndef USE_OSTREAM_ONLY
	///
    void Write(string &);
#endif
	///
    bool setArgumentIdx(int);
	///
    int  getArgumentIdx();
	///
    int  getMaxArgumentIdx();
	///
    int GetColumns();
	///
    void GetXY(int &, int &) const;
	///
    void SetFocus(int, int);
	///
    LyxArrayBase * GetData();
	///
    MathedRowSt * getRowSt() const { return args[idx].row; }
	///
    void SetData(LyxArrayBase *);
	///
    MathedTextCodes getTCode() { return tcode; }
	///
    bool Permit(short);
    
private:
	///
	MathMacroTemplate * tmplate;
	///
	struct MacroArgumentBase {
		/// Position of the macro
		int x, y;
		///
		MathedRowSt * row;
		///
		LyxArrayBase * array;
		///
		MacroArgumentBase() { x = y = 0;  array = 0; row = 0; }
	};
	MacroArgumentBase * args;
	///
	int idx;
	///
	int nargs;
	///
	MathedTextCodes tcode;
	///
	friend class MathMacroTemplate;
};


/// An argument
class MathMacroArgument: public MathParInset {
public:
    ///
    MathMacroArgument() { expnd_mode = false; number = 1;  SetType(LM_OT_MACRO_ARG); }
    ///
    MathMacroArgument(int);
    ///
	~MathMacroArgument() { lyxerr << "help, destroyme!" << endl; }
    ///
    MathedInset * Clone() { return this; }
	///
    void Metrics();
	///
    void draw(Painter &, int x, int baseline);
	///
    void Write(ostream &);
#ifndef USE_OSTREAM_ONLY
	///
    void Write(string &);
#endif
    ///
    void setNumber(int n) { number = n; }
    /// Is expanded or not
    void setExpand(bool e) { expnd_mode = e; }
    /// Is expanded or not
    bool getExpand() { return expnd_mode; }
    
private:
	///
    bool expnd_mode;
	///
    int number;
};


/// This class contains the macro definition
class MathMacroTemplate: public MathParInset {
public:
    /// A template constructor needs all the data
    MathMacroTemplate(char const *, int na = 0, int f = 0);
	///
    ~MathMacroTemplate();
	///
    void draw(Painter &, int, int);
	///
    void Metrics();
	///
    void WriteDef(ostream &);
#ifndef USE_OSTREAM_ONLY
	///
    void WriteDef(string &);
#endif
    /// useful for special insets
    void  setTCode(MathedTextCodes t) { tcode = t; }
    ///
    MathedTextCodes getTCode() { return tcode; }
    /// 
    void setArgument(LyxArrayBase *, int i= 0);
    /// Number of arguments
    int getNoArgs() { return nargs; }
    ///
    void GetMacroXY(int, int &, int &) const;
    ///
    MathParInset * getMacroPar(int) const;
    ///
    void SetMacroFocus(int &, int, int);
	///
    void setEditMode(bool);

    /// Replace the appropriate arguments with a specific macro's data
    void update(MathMacro * m = 0);
      
private:
    ///
    short flags;
    ///
    MathedTextCodes tcode;
    ///
    MathMacroArgument * args;
    ///
    int nargs;
    ///
    friend class MathMacro;
};
     

///
typedef MathMacro * MathMacroP;
///
typedef MathMacroTemplate * MathMacroTemplateP;

///
class MathMacroTable {
public:
	///
    MathMacroTable(int);
	///
    ~MathMacroTable();
	///
    void addTemplate(MathMacroTemplate *);
	///
    MathMacro * getMacro(char const *) const;
	///
    MathMacroTemplate * getTemplate(char const *) const;
	///
    void builtinMacros();
	///
    static MathMacroTable mathMTable;
	///
    static bool built;
    
private:
	///
    const int max_macros;
	///
    int num_macros;
	///
    MathMacroTemplateP * macro_table;
};



/*-----------------------  inlines  -------------------------*/

inline
bool MathMacro::setArgumentIdx(int i)
{
    if (i >= 0 && i < nargs) {
	idx = i;
	return true;
    } else
      return false;
}

inline
int  MathMacro::getArgumentIdx() 
{ 
    return idx; 
}

inline
int  MathMacro::getMaxArgumentIdx() 
{ 
    return nargs - 1; 
} 


inline
LyxArrayBase * MathMacro::GetData() 
{ 
    return args[idx].array; 
} 


inline
void MathMacro::SetData(LyxArrayBase * a)
{
   args[idx].array = a;
}


inline 
MathMacro * MathMacroTable::getMacro(char const * name) const
{
	MathMacroTemplate * mt = getTemplate(name);
	return (mt) ? new MathMacro(mt): 0;
}


#endif
