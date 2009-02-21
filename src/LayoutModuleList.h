// -*- C++ -*-
/**
 * \file ModuleList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LAYOUTMODULELIST_H
#define LAYOUTMODULELIST_H

#include <list>
#include <string>

namespace lyx {

class LayoutFile;

class LayoutModuleList {
public:
	///
	typedef std::list<std::string>::const_iterator const_iterator;
	///
	typedef std::list<std::string>::iterator iterator;
	///
	iterator begin() { return lml_.begin(); }
	///
	iterator end() { return lml_.end(); }
	///
	const_iterator begin() const { return lml_.begin(); }
	///
	const_iterator end() const { return lml_.end(); }
	///
	void clear() { lml_.clear(); }
	///
	bool empty() const { return lml_.empty(); }
	///
	iterator erase(iterator pos) { return lml_.erase(pos); }
	///
	iterator insert(iterator pos, std::string const & str)
		{ return lml_.insert(pos, str); }
	///
	void push_back(std::string const & str) { lml_.push_back(str); }
	/// 
	size_t size() const { return lml_.size(); }
	/// This is needed in GuiDocument. It seems better than an
	/// implicit conversion.
	std::list<std::string> const & list() const { return lml_; }
	/// Checks to make sure module's requriements are satisfied, that it does
	/// not conflict with already-present modules, isn't already loaded, etc.
	bool moduleCanBeAdded(std::string const & modName, 
			LayoutFile const * const lay) const;
	/// If the user changes the base class for a given document, then the
	/// associated module list has to be updated. This just calls
	/// (i)   removeBadModules()
	/// (ii)  addDefaultModules()
	/// (iii) checkModuleConsistency()
	/// in that order, for which see below.
	/// \param lay The new base class.
	/// \param removedModules Modules the user explicitly removed and so
	/// which should not be included.
	/// \return true if no changes had to be made, false if some did have
	/// to be made.
	bool adaptToBaseClass(LayoutFile const * const lay,
			std::list<std::string> removedModules);
private:
	/// Removes modules excluded by, provided by, etc, the base class.
	/// \param lay The document class against which to check.
	/// \return true, if modules were consistent, false if changes had
	/// to be made.
	bool removeBadModules(LayoutFile const * const lay);
	/// Adds default modules, if they're addable.
	/// \param lay The base class from which to get default modules.
	/// \param removedModules Modules the user has explicitly removed.
	void addDefaultModules(LayoutFile const * const lay,
			std::list<std::string> removedModules);
	/// Checks for consistency among modules: makes sure requirements
	/// are met, no modules exclude one another, etc, and resolves any
	/// such conflicts, leaving us with a consistent collection.
	/// \param lay The base class against which to check.
	/// \return true if modules were consistent, false if changes had
	/// to be made.
	bool checkModuleConsistency(LayoutFile const * const lay);
	///
	std::list<std::string> lml_;
};
}
#endif
