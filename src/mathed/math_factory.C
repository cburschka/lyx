#include <config.h>

#include "math_parser.h"
#include "math_bigopinset.h"
#include "math_binominset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_funcinset.h"
#include "math_funcliminset.h"
#include "math_fracinset.h"
#include "math_kerninset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_noglyphinset.h"
#include "math_notinset.h"
#include "math_rootinset.h"
#include "math_spaceinset.h"
#include "math_specialcharinset.h"
#include "math_sqrtinset.h"
#include "math_symbolinset.h"
#include "math_stackrelinset.h"


MathInset * createMathInset(latexkeys const * l)
{
	switch (l->token) {
	case LM_TK_NOGLYPH:
	case LM_TK_NOGLYPHB:
		return new MathNoglyphInset(l);
	case LM_TK_BIGSYM:
		return new MathBigopInset(l);
	case LM_TK_FUNCLIM:
		return new MathFuncLimInset(l);
	case LM_TK_SPECIAL:
		return new MathSpecialCharInset(l->id);
	case LM_TK_SYM:
	case LM_TK_CMSY:
	case LM_TK_CMM:
	case LM_TK_CMEX:
	case LM_TK_MSA:
	case LM_TK_MSB:
		return new MathSymbolInset(l);
	case LM_TK_STACK:
		return new MathStackrelInset;
	case LM_TK_KERN: 
		return new MathKernInset;
	case LM_TK_BINOM:
	case LM_TK_CHOOSE:
		return new MathBinomInset;
	case LM_TK_OVER:
	case LM_TK_FRAC:
		return new MathFracInset;
	case LM_TK_ATOP:
		return new MathFracInset(true);
	case LM_TK_NOT:
		return new MathNotInset;
	case LM_TK_SQRT:
		return new MathSqrtInset;
	case LM_TK_ROOT:
		return new MathRootInset;
	case LM_TK_DECORATION:
		return new MathDecorationInset(l);
	case LM_TK_SPACE:
		return new MathSpaceInset(l->id);
	case LM_TK_DOTS:
		return new MathDotsInset(l);
	}
	return new MathFuncInset(l->name);
}


MathInset * createMathInset(string const & s)
{
	latexkeys const * l = in_word_set(s);
	if (l) 
		return createMathInset(l);

	if (MathMacroTable::hasTemplate(s)) 
		return new MathMacro(MathMacroTable::provideTemplate(s));

	return new MathFuncInset(s);
}
