#include <config.h>

#include "math_parser.h"
#include "math_arrayinset.h"
#include "math_amsarrayinset.h"
#include "math_binominset.h"
#include "math_boxinset.h"
#include "math_casesinset.h"
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
#include "math_sizeinset.h"
#include "math_spaceinset.h"
#include "math_splitinset.h"
#include "math_specialcharinset.h"
#include "math_sqrtinset.h"
#include "math_stackrelinset.h"
#include "math_substackinset.h"
#include "math_symbolinset.h"
#include "math_undersetinset.h"
#include "math_unknowninset.h"
#include "math_xarrowinset.h"
#include "math_xymatrixinset.h"
#include "math_xyarrowinset.h"


MathAtom createMathInset(latexkeys const * l)
{
	switch (l->token) {
	case LM_TK_FUNCLIM:
		return MathAtom(new MathFuncLimInset(l->name));
	case LM_TK_SPECIAL:
		return MathAtom(new MathSpecialCharInset(static_cast<char>(l->id)));
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
	case LM_TK_UNDERSET:
		return MathAtom(new MathUndersetInset);
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
	case LM_TK_BOX:
		return MathAtom(new MathBoxInset(l->name));
	case LM_TK_FUNC:
		return MathAtom(new MathFuncInset(l->name));
	case LM_TK_STY:
		return MathAtom(new MathSizeInset(l));
	default:
		return MathAtom(new MathUnknownInset(l->name));
	}
}


MathAtom createMathInset(string const & s)
{
	//cerr << "creating inset with name: '" << s << "'\n";
	if (s.size() == 2 && s[0] == '#' && s[1] >= '1' && s[1] <= '9')
		return MathAtom(new MathMacroArgument(s[1] - '0'));

	if (s.size() == 3 && s[0] == '\\' && s[1] == '#'
			&& s[2] >= '1' && s[2] <= '9')
		return MathAtom(new MathMacroArgument(s[2] - '0'));

	if (s == "xymatrix")
		return MathAtom(new MathXYMatrixInset);

	if (s == "xrightarrow")
		return MathAtom(new MathXArrowInset(s));

	if (s == "split")
		return MathAtom(new MathSplitInset(1));

	if (s == "cases")
		return MathAtom(new MathCasesInset);

	if (s == "substack")
		return MathAtom(new MathSubstackInset);

	if (s == "subarray" || s == "array")
		return MathAtom(new MathArrayInset(s, 1, 1));

	if (s == "pmatrix" || s == "bmatrix" || s == "vmatrix" || s == "Vmatrix") 
		return MathAtom(new MathAMSArrayInset(s));

	latexkeys const * l = in_word_set(s);
	if (l)
		return createMathInset(l);

	if (MathMacroTable::has(s)) 
		return MathAtom(new MathMacro(s));

	//cerr << "creating inset 2 with name: '" << s << "'\n";
	return MathAtom(new MathUnknownInset(s));
}
