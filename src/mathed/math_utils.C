/* 
 *  File:        math_utils.C
 *  Purpose:     X independent general mathed routines
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     August 1996
 *  
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *  License: GNU GPL version 2 or later
 */

#include <config.h>

#include <algorithm>

#include "math_defs.h"
#include "symbol_def.h"

using std::sort;
using std::lower_bound;

namespace {

// This table includes all binary operators and relations
struct binary_op_pair {
	short id;
	short isrel;
};


binary_op_pair binary_op_table[] = {
      { LM_leq, LMB_RELATION }, { LM_geq, LMB_RELATION }, 
      { LM_equiv, LMB_RELATION }, { LM_models, LMB_RELATION }, 
      { LM_prec, LMB_RELATION }, { LM_succ, LMB_RELATION }, 
      { LM_sim, LMB_RELATION }, { LM_perp, LMB_RELATION }, 
      { LM_preceq, LMB_RELATION }, { LM_succeq, LMB_RELATION }, 
      { LM_simeq, LMB_RELATION }, { LM_mid, LMB_RELATION }, 
      { LM_ll, LMB_RELATION }, { LM_gg, LMB_RELATION }, 
      { LM_asymp, LMB_RELATION }, { LM_parallel, LMB_RELATION }, 
      { LM_subset, LMB_RELATION }, { LM_supset, LMB_RELATION }, 
      { LM_approx, LMB_RELATION }, { LM_smile, LMB_RELATION }, 
      { LM_subseteq, LMB_RELATION }, { LM_supseteq, LMB_RELATION }, 
      { LM_cong, LMB_RELATION }, { LM_frown, LMB_RELATION }, 
      { LM_sqsubseteq, LMB_RELATION }, { LM_sqsupseteq, LMB_RELATION }, 
      { LM_doteq, LMB_RELATION }, { LM_neq, LMB_RELATION }, 
      { LM_in, LMB_RELATION }, { LM_ni, LMB_RELATION }, 
      { LM_propto, LMB_RELATION }, { LM_notin, LMB_RELATION }, 
      { LM_vdash, LMB_RELATION }, { LM_dashv, LMB_RELATION }, 
      { LM_bowtie, LMB_RELATION },
      { LM_pm, LMB_OPERATOR }, { LM_cap, LMB_OPERATOR }, 
      { LM_diamond, LMB_OPERATOR }, { LM_oplus, LMB_OPERATOR },
      { LM_mp, LMB_OPERATOR }, { LM_cup, LMB_OPERATOR }, 
      { LM_bigtriangleup, LMB_OPERATOR }, { LM_ominus, LMB_OPERATOR },
      { LM_times, LMB_OPERATOR }, { LM_uplus, LMB_OPERATOR }, 
      { LM_bigtriangledown, LMB_OPERATOR }, { LM_otimes, LMB_OPERATOR },
      { LM_div, LMB_OPERATOR }, { LM_sqcap, LMB_OPERATOR }, 
      { LM_triangleright, LMB_OPERATOR }, { LM_oslash, LMB_OPERATOR },
      { LM_cdot, LMB_OPERATOR }, { LM_sqcup, LMB_OPERATOR }, 
      { LM_triangleleft, LMB_OPERATOR }, { LM_odot, LMB_OPERATOR },
      { LM_star, LMB_OPERATOR }, { LM_vee, LMB_OPERATOR }, 
      { LM_amalg, LMB_OPERATOR }, { LM_bigcirc, LMB_OPERATOR },
      { LM_setminus, LMB_OPERATOR }, { LM_wedge, LMB_OPERATOR }, 
      { LM_dagger, LMB_OPERATOR }, { LM_circ, LMB_OPERATOR },
      { LM_bullet, LMB_OPERATOR }, { LM_wr, LMB_OPERATOR }, 
      { LM_ddagger, LMB_OPERATOR }
};


struct compara {
	// used by sort and lower_bound
	inline
	int operator()(binary_op_pair const & a,
		       binary_op_pair const & b) const {
		return a.id < b.id;
	}
};

} // namespace anon


int MathedLookupBOP(short id)
{
	static int const bopCount =
		sizeof(binary_op_table) / sizeof(binary_op_pair);
	static bool issorted = false;
	
	if (!issorted) {
		sort(binary_op_table, binary_op_table + bopCount, compara());
		issorted = true;
	}

	binary_op_pair search_elem = { id, 0 };
	
	binary_op_pair * res = lower_bound(binary_op_table,
					   binary_op_table + bopCount,
					   search_elem, compara());
	if (res != binary_op_table + bopCount && res->id == id)
		return res->isrel;
	else
		return LMB_NONE;
}
