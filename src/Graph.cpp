/**
 * \file Graph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur (original code)
 * \author Richard Kimberly Heck (re-implementation)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Graph.h"
#include "Format.h"

#include "support/debug.h"
#include "support/lassert.h"

using namespace std;

namespace lyx {


bool Graph::bfs_init(int s, bool clear_visited, queue<int> & Q)
{
	if (s < 0)
		return false;

	if (!Q.empty())
		Q = queue<int>();

	if (clear_visited) {
		vector<Vertex>::iterator it = vertices_.begin();
		vector<Vertex>::iterator en = vertices_.end();
		for (; it != en; ++it)
			it->visited = false;
	}
	if (!vertices_[s].visited) {
		Q.push(s);
		vertices_[s].visited = true;
	}
	return true;
}


Graph::EdgePath const
	Graph::getReachableTo(int to, bool clear_visited)
{
	EdgePath result;
	queue<int> Q;
	if (!bfs_init(to, clear_visited, Q))
		return result;

	// Here's the logic, which is shared by the other routines.
	// Q holds a list of nodes we have been able to reach (in this
	// case, reach backwards). It is initialized to the current node
	// by bfs_init, and then we recurse, adding the nodes we can reach
	// from the current node as we go. That makes it a breadth-first
	// search.
	while (!Q.empty()) {
		int const current = Q.front();
		Q.pop();
		if (current != to || theFormats().get(to).name() != "lyx")
			result.push_back(current);

		vector<Arrow *>::iterator it = vertices_[current].in_arrows.begin();
		vector<Arrow *>::iterator const end = vertices_[current].in_arrows.end();
		for (; it != end; ++it) {
			const int cv = (*it)->from;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q.push(cv);
			}
		}
	}

	return result;
}


Graph::EdgePath const
	Graph::getReachable(int from, bool only_viewable,
		bool clear_visited, set<int> excludes)
{
	EdgePath result;
	queue<int> Q;
	if (!bfs_init(from, clear_visited, Q))
		return result;

	while (!Q.empty()) {
		int const current = Q.front();
		Q.pop();
		Format const & format = theFormats().get(current);
		if (!only_viewable || !format.viewer().empty())
			result.push_back(current);
		else if (format.isChildFormat()) {
			Format const * const parent =
				theFormats().getFormat(format.parentFormat());
			if (parent && !parent->viewer().empty())
				result.push_back(current);
		}

		vector<Arrow *>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<Arrow *>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = (*cit)->to;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				if (excludes.find(cv) == excludes.end())
					Q.push(cv);
			}
		}
	}

	return result;
}


bool Graph::isReachable(int from, int to)
{
	if (from == to)
		return true;

	queue<int> Q;
	if (to < 0 || !bfs_init(from, true, Q))
		return false;

	while (!Q.empty()) {
		int const current = Q.front();
		Q.pop();
		if (current == to)
			return true;

		vector<Arrow *>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<Arrow *>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = (*cit)->to;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q.push(cv);
			}
		}
	}

	return false;
}


Graph::EdgePath const Graph::getPath(int from, int to)
{
	if (from == to)
		return EdgePath();

	queue<int> Q;
	if (to < 0 || !bfs_init(from, true, Q))
		return EdgePath();

	vector<EdgePath> pathes;
	pathes.resize(vertices_.size());
	while (!Q.empty()) {
		int const current = Q.front();
		Q.pop();

		vector<Arrow *>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<Arrow *>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = (*cit)->to;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q.push(cv);
				// NOTE If we wanted to collect all the paths, then
				// we just need to collect them here and not worry
				// about "visited".
				EdgePath lastpath = pathes[(*cit)->from];
				lastpath.push_back((*cit)->id);
				pathes[cv] = lastpath;
			}
			if (cv == to) {
				return pathes[cv];
			}
		}
	}
	// failure
	return EdgePath();
}


void Graph::init(int size)
{
	vertices_ = vector<Vertex>(size);
	arrows_.clear();
	numedges_ = 0;
}


void Graph::addEdge(int from, int to)
{
	arrows_.push_back(Arrow(from, to, numedges_));
	numedges_++;
	Arrow * ar = &(arrows_.back());
	vertices_[to].in_arrows.push_back(ar);
	vertices_[from].out_arrows.push_back(ar);
}


// At present, we do not need this debugging code, but
// I am going to leave it here in case we need it again.
#if 0
void Graph::dumpGraph() const
{
	vector<Vertex>::const_iterator it = vertices_.begin();
	vector<Vertex>::const_iterator en = vertices_.end();
	for (; it != en; ++it) {
		LYXERR0("Next vertex...");
		LYXERR0("In arrows...");
		std::vector<Arrow *>::const_iterator iit = it->in_arrows.begin();
		std::vector<Arrow *>::const_iterator ien = it->in_arrows.end();
		for (; iit != ien; ++iit)
			LYXERR0("From " << (*iit)->from << " to " << (*iit)->to);
		LYXERR0("Out arrows...");
		iit = it->out_arrows.begin();
		ien = it->out_arrows.end();
		for (; iit != ien; ++iit)
			LYXERR0("From " << (*iit)->from << " to " << (*iit)->to);
	}
}
#endif


} // namespace lyx
