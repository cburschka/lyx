/**
 * \file Graph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Graph.h"
#include "Format.h"

#include <algorithm>

using namespace std;

namespace lyx {


bool Graph::bfs_init(int s, bool clear_visited)
{
	if (s < 0)
		return false;

	Q_ = queue<int>();

	if (clear_visited)
		fill(visited_.begin(), visited_.end(), false);
	if (visited_[s] == false) {
		Q_.push(s);
		visited_[s] = true;
	}
	return true;
}


vector<int> const
	Graph::getReachableTo(int target, bool clear_visited)
{
	vector<int> result;
	if (!bfs_init(target, clear_visited))
		return result;

	// Here's the logic, which is shared by the other routines.
	// Q_ holds a list of nodes we have been able to reach (in this
	// case, reach backwards). It is initailized to the current node
	// by bfs_init, and then we recurse, adding the nodes we can reach
	// from the current node as we go.
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		if (current != target || formats.get(target).name() != "lyx")
			result.push_back(current);

		vector<int>::iterator it = vertices_[current].in_vertices.begin();
		vector<int>::iterator end = vertices_[current].in_vertices.end();
		for (; it != end; ++it) {
			if (!visited_[*it]) {
				visited_[*it] = true;
				Q_.push(*it);
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

		vector<OutEdge>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<OutEdge>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = cit->vertex;
			if (!visited_[cv]) {
				visited_[cv] = true;
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

		vector<OutEdge>::const_iterator cit =
			vertices_[current].out_arrows.begin();
		vector<OutEdge>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = cit->vertex;
			if (!visited_[cv]) {
				visited_[cv] = true;
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

	// pair<vertex, edge>
	vector<pair<int, int> > prev(vertices_.size());

	bool found = false;
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		if (current == to) {
			found = true;
			break;
		}

		vector<OutEdge>::const_iterator const beg =
			vertices_[current].out_arrows.begin();
		vector<OutEdge>::const_iterator cit = beg;
		vector<OutEdge>::const_iterator end =
			vertices_[current].out_arrows.end();
		for (; cit != end; ++cit) {
			int const cv = cit->vertex;
			if (!visited_[cv]) {
				visited_[cv] = true;
				Q_.push(cv);
				// FIXME This will not do for finding multiple paths.
				// Perhaps we need a vector, or a set.
				prev[cv] = pair<int, int>(current, cit->edge);
			}
		}
	}
	if (!found)
		return path;

	while (to != from) {
		path.push_back(prev[to].second);
		to = prev[to].first;
	}
	reverse(path.begin(), path.end());
	return path;
}


void Graph::init(int size)
{
	vertices_ = vector<Vertex>(size);
	visited_.resize(size);
	numedges_ = 0;
}


void Graph::addEdge(int from, int to)
{
	vertices_[to].in_vertices.push_back(from);
	vertices_[from].out_arrows.push_back(OutEdge(to, numedges_++));
}


vector<Graph::Vertex> Graph::vertices_;


} // namespace lyx
