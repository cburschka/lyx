// -*- C++ -*-

#ifndef SHARECONTAINER_H
#define SHARECONTAINER_H

#include <vector>
#include <algorithm>
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

///
template<class Share>
class ShareContainer : public noncopyable {
public:
	///
	typedef std::vector<boost::shared_ptr<Share> > Params;
	///
	Params::value_type
	get(Share const & ps) const {
		// First see if we already have this ps in the container
		Params::iterator it = params.begin();
		Params::iterator end = params.end();
		for (; it != end; ++it) {
			if (ps == *(*it).get())
				break;
		}
		Params::value_type tmp;
		if (it == end) {
			// ok we don't have it so we should
			// insert it.
			tmp.reset(new Share(ps));
			params.push_back(tmp);
			// We clean here. This can cause us to have
			// some (one) unique elemements some times
			// but we should gain a lot in speed.
			clean();
			std::sort(params.rbegin(), params.rend(), comp());
		} else {
			// yes we have it already
			tmp = *it;
		}
		return tmp;
	}
private:
	///
	struct comp {
		int operator()(Params::value_type const & p1,
			       Params::value_type const & p2) {
			return p1.use_count() < p2.use_count();
		}
	};
	///
	struct isUnique {
		bool operator()(Params::value_type const & p) const {
			return p.unique();
		}
	};

	///
	void clean() const {
		// Remove all unique items. (i.e. entries that only
		// exists in the conatainer and does not have a
		// corresponding paragrah.
		Params::iterator it = std::remove_if(params.begin(),
						     params.end(),
						     isUnique());
		params.erase(it, params.end());
	}
	
	///
	mutable Params params;
};
#endif
