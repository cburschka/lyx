// -*- C++ -*-
#ifndef MATHMACROTABLE
#define MATHMACROTABLE

#include <vector>
#include "LString.h"

class MathMacroTemplate;
class MathMacro;

///
class MathMacroTable {
public:
    ///
    void addTemplate(MathMacroTemplate *);
    ///
    MathMacro * getMacro(string const &) const;
    ///
    MathMacroTemplate * getTemplate(string const &) const;
    ///
    void builtinMacros();
    ///
    static MathMacroTable mathMTable;
    ///
    static bool built;
private:
    ///
    typedef std::vector<MathMacroTemplate *> table_type;
    ///
    typedef table_type::size_type size_type;
    ///
    table_type macro_table;
};
#endif
