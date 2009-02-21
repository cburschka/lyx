// -*- C++ -*-
/**
 * \file ModuleList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LayoutModuleList.h"

#include "LayoutFile.h"
#include "ModuleList.h"

#include "support/debug.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

namespace lyx {


// the previous document class may have loaded some modules that the
// new one excludes, and the new class may provide, etc, some that
// conflict with ones that were already loaded. So we need to go 
// through the list and fix everything. I suppose there are various
// ways this could be done, but the following seems to work at the 
// moment. (Thanks to Philippe Charpentier for helping work out all 
// the bugs---rgh.)
bool LayoutModuleList::adaptToBaseClass(LayoutFile const * const lay,
		std::list<string> const & removedModules)
{
	// first, we remove any modules the new document class itself provides,
	// those it excludes, and those that conflict with ones it excludes.
	// this has to be done first because, otherwise, a module we're about
	// to remove could prevent a default module from being added.
	bool retval = removeBadModules(lay);
	// next, we add any default modules the new class provides.
	addDefaultModules(lay, removedModules);
	// finally, we perform a general consistency check on the set of
	// loaded modules. it's a hassle that we have to do this now, since
	// we just went through them a bit ago, but things might have changed
	// with the loading of the default modules.
	retval = (checkModuleConsistency(lay) || retval);
	return retval;
}


bool LayoutModuleList::moduleCanBeAdded(string const & modName, 
		LayoutFile const * const lay) const
{
	// Is the module already present?
	const_iterator it = begin();
	const_iterator const en = end();
	for (; it != en; it++)
		if (*it == modName) 
			return false;

	LyXModule const * const lm = moduleList[modName];
	if (!lm)
		return true;

	// Is this module explicitly excluded by the document class?
	const_iterator const exclmodstart = lay->excludedModules().begin();
	const_iterator const exclmodend = lay->excludedModules().end();
	if (find(exclmodstart, exclmodend, modName) != exclmodend)
		return false;

	// Is this module already provided by the document class?
	const_iterator const provmodstart = lay->providedModules().begin();
	const_iterator const provmodend = lay->providedModules().end();
	if (find(provmodstart, provmodend, modName) != provmodend)
		return false;

	// Check for conflicts with used modules
	// first the provided modules...
	const_iterator provmodit = provmodstart;
	for (; provmodit != provmodend; ++provmodit) {
		if (!LyXModule::areCompatible(modName, *provmodit))
			return false;
	}
	// and then the selected modules
	const_iterator mit = begin();
	const_iterator const men = end();
	for (; mit != men; ++mit)
		if (!LyXModule::areCompatible(modName, *mit))
			return false;

	// Check whether some required module is available
	vector<string> const reqs = lm->getRequiredModules();
	if (reqs.empty())
		return true;

	mit = begin(); // reset
	vector<string>::const_iterator rit = reqs.begin();
	vector<string>::const_iterator ren = reqs.end();
	bool foundone = false;
	for (; rit != ren; ++rit) {
		if (find(mit, men, *rit) != men || 
		    find(provmodstart, provmodend, *rit) != provmodend) {
			foundone = true;
			break;
		}
	}

	return foundone;
}


void LayoutModuleList::addDefaultModules(LayoutFile const * const lay,
		std::list<string> removedModules)
{
	LayoutModuleList mods = lay->defaultModules();
	const_iterator mit = mods.begin();
	const_iterator const men = mods.end();

	// We want to insert the default modules at the beginning of
	// the list, but also to insert them in the correct order.
	// The obvious thing to do would be to collect them and then
	// insert them, but that doesn't work because a later default
	// module may require an earlier one, and then the test below
	//     moduleCanBeAdded(modname)
	// will fail. So we have to do it a more complicated way.
	iterator insertpos = begin();
	int numinserts = 0;

	for (; mit != men; mit++) {
		string const & modName = *mit;
		// make sure the user hasn't removed it
		if (find(removedModules.begin(), removedModules.end(), modName) !=
		    removedModules.end()) {
			LYXERR(Debug::TCLASS, "Default module `" << modName << 
					"' not added because removed by user.");
			continue;
		}

		if (!moduleCanBeAdded(modName, lay)) {
			// FIXME This could be because it's already present, so we should
			// probably return something indicating that.
			LYXERR(Debug::TCLASS, "Default module `" << modName << 
					"' could not be added.");
			continue;
		}
		LYXERR(Debug::TCLASS, "Default module `" << modName << "' added.");
		insert(insertpos, modName);
		// now we reset insertpos
		++numinserts;
		insertpos = begin();
		advance(insertpos, numinserts);
	}
}


bool LayoutModuleList::removeBadModules(LayoutFile const * const lay)
{
	// we'll write a new list of modules, since we can't just remove them,
	// as that would invalidate our iterators
	LayoutModuleList oldModules = *this;
	clear();

	LayoutModuleList const & provmods = lay->providedModules();
	LayoutModuleList const & exclmods = lay->excludedModules();
	bool consistent = true; // set to false if we have to do anything

	const_iterator oit = oldModules.begin();
	const_iterator const oen = oldModules.end();
	for (; oit != oen; ++oit) {
		string const & modname = *oit;
		// skip modules that the class provides
		if (find(provmods.begin(), provmods.end(), modname) != provmods.end()) {
			LYXERR0("Module `" << modname << "' dropped because provided by document class.");
			consistent = false;
			continue;
		}
		// are we excluded by the document class?
		if (find(exclmods.begin(), exclmods.end(), modname) != exclmods.end()) {
			LYXERR0("Module `" << modname << "' dropped because excluded by document class.");
			consistent = false;
			continue;
		}
		// determine whether some provided module excludes us or we exclude it
		const_iterator pit = provmods.begin();
		const_iterator const pen = provmods.end();
		bool excluded = false;
		for (; !excluded && pit != pen; ++pit) {
			if (!LyXModule::areCompatible(modname, *pit)) {
				LYXERR0("Module " << modname << 
						" dropped becuase it conflicts with provided module `" << *pit << "'.");
				consistent = false;
				excluded = true;
			}
		}
		if (excluded)
			continue;
		push_back(modname);
	}
	return consistent;
}


// Perform a consistency check on the set of modules. We need to make
// sure that none of the modules exclude each other and that requires
// are satisfied.
bool LayoutModuleList::checkModuleConsistency(LayoutFile const * const lay) 
{
	bool consistent = true;
	LayoutModuleList oldModules = *this;
	clear();
	const_iterator oit = oldModules.begin();
	const_iterator const oen = oldModules.end();
	LayoutModuleList const & provmods = lay->providedModules();
	for (; oit != oen; ++oit) {
		string const & modname = *oit;
		bool excluded = false;
		// Determine whether some prior module excludes us, or we exclude it
		const_iterator lit = begin();
		const_iterator const len = end();
		for (; !excluded && lit != len; ++lit) {
			if (!LyXModule::areCompatible(modname, *lit)) {
				consistent = false;
				LYXERR0("Module " << modname << 
						" dropped because it is excluded by prior module " << *lit);
				excluded = true;
			}
		}

		if (excluded)
			continue;

		// determine whether some provided module or some prior module
		// satisfies our requirements
		LyXModule const * const oldmod = moduleList[modname];
		if (!oldmod) {
			LYXERR0("Default module " << modname << 
					" added although it is unavailable and can't check requirements.");
			continue;
		}
			
		vector<string> const & reqs = oldmod->getRequiredModules();
		if (!reqs.empty()) {
			// we now set excluded to true, meaning that we haven't
			// yet found a required module.
			excluded = true;
			vector<string>::const_iterator rit  = reqs.begin();
			vector<string>::const_iterator const ren = reqs.end();
			for (; rit != ren; ++rit) {
				string const reqmod = *rit;
				if (find(provmods.begin(), provmods.end(), reqmod) != 
						provmods.end()) {
					excluded = false;
					break;
				}
				if (find(begin(), end(), reqmod) != end()) {
					excluded = false;
					break;
				}
			}
		}
		if (excluded) {
			consistent = false;
			LYXERR0("Module " << modname << " dropped because requirements not met.");
		} else {
			LYXERR(Debug::TCLASS, "Module " << modname << " passed consistency check.");
			push_back(modname);
		}
	}
	return consistent;
}

} // namespace lyx
