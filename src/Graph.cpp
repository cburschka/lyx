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

	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		if (current != target || formats.get(target).name() != "lyx") {
			result.push_back(current);
		}

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
		int const i = Q_.front();
		Q_.pop();
		Format const & format = formats.get(i);
		if (!only_viewable || !format.viewer().empty())
			result.push_back(i);
		else if (format.isChildFormat()) {
			Format const * const parent =
				formats.getFormat(format.parentFormat());
			if (parent && !parent->viewer().empty())
				result.push_back(i);
		}

		vector<int>::const_iterator cit =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit)
			if (!visited_[*cit]) {
				visited_[*cit] = true;
				Q_.push(*cit);
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

		vector<int>::const_iterator cit =
			vertices_[current].out_vertices.begin();
		vector<int>::const_iterator end =
			vertices_[current].out_vertices.end();
		for (; cit != end; ++cit) {
			if (!visited_[*cit]) {
				visited_[*cit] = true;
				Q_.push(*cit);
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

	vector<int> prev_edge(formats.size());
	vector<int> prev_vertex(formats.size());

	bool found = false;
	while (!Q_.empty()) {
		int const current = Q_.front();
		Q_.pop();
		if (current == to) {
			found = true;
			break;
		}

		vector<int>::const_iterator const beg =
			vertices_[current].out_vertices.begin();
		vector<int>::const_iterator cit = beg;
		vector<int>::const_iterator end =
			vertices_[current].out_vertices.end();
		for (; cit != end; ++cit)
			if (!visited_[*cit]) {
				int const j = *cit;
				visited_[j] = true;
				Q_.push(j);
				int const k = cit - beg;
				prev_edge[j] = vertices_[current].out_edges[k];
				prev_vertex[j] = current;
			}
	}
	if (!found)
		return path;

	while (to != from) {
		path.push_back(prev_edge[to]);
		to = prev_vertex[to];
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
	vertices_[from].out_vertices.push_back(to);
	vertices_[from].out_edges.push_back(numedges_++);
}


vector<Graph::Vertex> Graph::vertices_;


} // namespace lyx
