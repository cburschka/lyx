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


void Graph::clearMarks()
{
	Arrows::iterator it = arrows_.begin();
	Arrows::iterator const en = arrows_.end();
	for (; it != en; ++it)
		it->marked = false;
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
	EdgePath path;
	if (from == to)
		return path;

	if (to < 0 || !bfs_init(from))
		return path;

	// In effect, the way this works is that we construct a sub-graph
	// by starting at "from" and following the arrows outward. Instead
	// of actually constructing a sub-graph, though, we "mark" the
	// arrows we traverse as we go. Once we hit "to", we abort the 
	// marking process and then call getMarkedPath() to reconstruct
	// the marked path.
	bool found = false;
	clearMarks();
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();

		vector<Arrow *>::const_iterator const beg =
			vertices_[current].out_arrows.begin();
		vector<Arrow *>::const_iterator cit = beg;
		vector<Arrow *>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = (*cit)->to;
			if (!vertices_[cv].visited) {
				vertices_[cv].visited = true;
				Q_.push(cv);
				(*cit)->marked = true;
			}
			if (cv == to) {
				found = true;
				break;
			}
		}
	}
	if (!found)
		return path;

	getMarkedPath(from, to, path);
	return path;
}


// We assume we have marked the graph, as in getPath(). We also
// assume that we have done so in such a way as to guarantee a
// marked path from "from" to "to".
// We then start at "to" and find the arrow leading to it that
// has been marked. We add that to the path we are constructing,
// step back on that arrow, and continue the process (i.e., recurse).
void Graph::getMarkedPath(int from, int to, EdgePath & path) {
	if (from == to) {
		reverse(path.begin(), path.end());
		return;
	}
	// find marked in_arrow
	vector<Arrow *>::const_iterator it = vertices_[to].in_arrows.begin();
	vector<Arrow *>::const_iterator en = vertices_[to].in_arrows.end();
	for (; it != en; ++it)
		if ((*it)->marked) 
			break;
	if (it == en) {
		LASSERT(false, /* */);
		return;
	}
	int const newnode = (*it)->from;
	path.push_back(newnode);
	getMarkedPath(from, newnode, path);
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


} // namespace lyx
