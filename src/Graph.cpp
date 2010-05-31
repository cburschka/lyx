/**
 * \file Graph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur (original code)
 * \author Richard Heck (re-implementation)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Graph.h"
#include "Format.h"

#include "support/debug.h"
#include "support/lassert.h"

#include <algorithm>

using namespace std;

namespace lyx {


bool Graph::bfs_init(int s, bool clear_visited)
{
	if (s < 0)
		return false;

	Q_ = queue<int>();

	if (clear_visited) {
		vector<Vertex>::iterator it = vertices_.begin();
		vector<Vertex>::iterator en = vertices_.end();
		for (; it != en; ++it)
			it->visited = false;
	}
	if (!vertices_[s].visited) {
		Q_.push(s);
		vertices_[s].visited = true;
	}
	return true;
}


void Graph::clearPaths()
{
	vector<Vertex>::iterator it = vertices_.begin();
	vector<Vertex>::iterator en = vertices_.end();
	for (; it != en; ++it)
		it->path.clear();
}


vector<int> const
	Graph::getReachableTo(int target, bool clear_visited)
{
	vector<int> result;
	if (!bfs_init(target, clear_visited))
		return result;

	// Here's the logic, which is shared by the other routines.
	// Q_ holds a list of nodes we have been able to reach (in this
	// case, reach backwards). It is initialized to the current node
	// by bfs_init, and then we recurse, adding the nodes we can reach
	// from the current node as we go. That makes it a breadth-first
	// search.
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		if (current != target || formats.get(target).name() != "lyx")
			result.push_back(current);

		vector<Arrow *>::iterator it = vertices_[current].in_arrows.begin();
		vector<Arrow *>::iterator const end = vertices_[current].in_arrows.end();
		for (; it != end; ++it) {
			const int cv = (*it)->from;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q_.push(cv);
			}
		}
	}

	return result;
}


vector<int> const
	Graph::getReachable(int from, bool only_viewable,
		bool clear_visited)
{
	vector<int> result;
	if (!bfs_init(from, clear_visited))
		return result;

	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		Format const & format = formats.get(current);
		if (!only_viewable || !format.viewer().empty())
			result.push_back(current);
		else if (format.isChildFormat()) {
			Format const * const parent =
				formats.getFormat(format.parentFormat());
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
				Q_.push(cv);
			}
		}
	}

	return result;
}


bool Graph::isReachable(int from, int to)
{
	if (from == to)
		return true;

	if (to < 0 || !bfs_init(from))
		return false;

	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
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
				Q_.push(cv);
			}
		}
	}

	return false;
}


Graph::EdgePath const Graph::getPath(int from, int to)
{
	static const EdgePath path;
	if (from == to)
		return path;

	if (to < 0 || !bfs_init(from))
		return path;

	clearPaths();
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();

		vector<Arrow *>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<Arrow *>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = (*cit)->to;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q_.push(cv);
				// NOTE If we wanted to collect all the paths, then
				// we just need to collect them here and not worry
				// about "visited".
				EdgePath lastpath = vertices_[(*cit)->from].path;
				lastpath.push_back((*cit)->id);
				vertices_[cv].path = lastpath;
			}
			if (cv == to) {
				return vertices_[cv].path;
			}
		}
	}
	// failure
	return path;
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
