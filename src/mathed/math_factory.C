#include <config.h>

#include "math_parser.h"
#include "math_binominset.h"
//#include "math_boxinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_funcinset.h"
#include "math_funcliminset.h"
#include "math_fracinset.h"
#include "math_kerninset.h"
#include "math_lefteqninset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macroarg.h"
#include "math_notinset.h"
#include "math_rootinset.h"
#include "math_spaceinset.h"
#include "math_specialcharinset.h"
#include "math_sqrtinset.h"
#include "math_symbolinset.h"
#include "math_stackrelinset.h"


MathAtom createMathInset(latexkeys const * l)
{
	switch (l->token) {
	case LM_TK_FUNCLIM:
		return MathAtom(new MathFuncLimInset(l));
	case LM_TK_SPECIAL:
		return MathAtom(new MathSpecialCharInset(l->id));
	case LM_TK_SYM:
	case LM_TK_CMR:
	case LM_TK_CMSY:
	case LM_TK_CMM:
	case LM_TK_CMEX:
	case LM_TK_MSA:
	case LM_TK_MSB:
		return MathAtom(new MathSymbolInset(l));
	case LM_TK_STACK:
		return MathAtom(new MathStackrelInset);
	case LM_TK_KERN: 
		return MathAtom(new MathKernInset);
	case LM_TK_BINOM:
	case LM_TK_CHOOSE:
		return MathAtom(new MathBinomInset);
	case LM_TK_OVER:
	case LM_TK_FRAC:
		return MathAtom(new MathFracInset);
	case LM_TK_ATOP:
		return MathAtom(new MathFracInset(true));
	case LM_TK_NOT:
		return MathAtom(new MathNotInset);
	case LM_TK_LEFTEQN:
		return MathAtom(new MathLefteqnInset);
	case LM_TK_SQRT:
		return MathAtom(new MathSqrtInset);
	case LM_TK_ROOT:
		return MathAtom(new MathRootInset);
	case LM_TK_DECORATION:
		return MathAtom(new MathDecorationInset(l->name));
	case LM_TK_SPACE:
		return MathAtom(new MathSpaceInset(l->id));
	case LM_TK_DOTS:
		return MathAtom(new MathDotsInset(l->name));
	//case LM_TK_BOX:
	//	return MathAtom(new MathBoxInset(l->name));
	}
	return MathAtom(new MathFuncInset(l->name));
}


MathAtom createMathInset(string const & s)
{
	//cerr << "creating inset with name: '" << s << "'\n";
	if (s.size() == 2 && s[0] == '#' && s[1] >= '1' && s[1] <= '9')
		return MathAtom(new MathMacroArgument(s[1] - '0'));

	if (s.size() == 3 && s[0] == '\\' && s[1] == '#' && s[2] >= '1' && s[2] <= '9')
		return MathAtom(new MathMacroArgument(s[2] - '0'));

	latexkeys const * l = in_word_set(s);
	if (l)
		return createMathInset(l);

	if (MathMacroTable::has(s)) 
		return MathAtom(new MathMacro(s));

	return MathAtom(new MathFuncInset(s));
}
