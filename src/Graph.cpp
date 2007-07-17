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

using std::vector;
using std::reverse;
using std::fill;


namespace lyx {


int Graph::bfs_init(int s, bool clear_visited)
{
	if (s < 0)
		return s;

	Q_ = std::queue<int>();

	if (clear_visited)
		fill(visited_.begin(), visited_.end(), false);
	if (visited_[s] == false) {
		Q_.push(s);
		visited_[s] = true;
	}
	return s;
}


vector<int> const
Graph::getReachableTo(int target, bool clear_visited)
{
	vector<int> result;
	int const s = bfs_init(target, clear_visited);
	if (s < 0)
		return result;

	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i != s || formats.get(target).name() != "lyx") {
			result.push_back(i);
		}

		vector<int>::iterator it = vertices_[i].in_vertices.begin();
		vector<int>::iterator end = vertices_[i].in_vertices.end();
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
	if (bfs_init(from, clear_visited) < 0)
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

	int const s = bfs_init(from);
	if (s < 0 || to < 0)
		return false;

	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i == to)
			return true;

		vector<int>::const_iterator cit =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit) {
			if (!visited_[*cit]) {
				visited_[*cit] = true;
				Q_.push(*cit);
			}
		}
	}

	return false;
}


Graph::EdgePath const
Graph::getPath(int from, int t)
{
	EdgePath path;
	if (from == t)
		return path;

	int const s = bfs_init(from);
	if (s < 0 || t < 0)
		return path;

	vector<int> prev_edge(formats.size());
	vector<int> prev_vertex(formats.size());

	bool found = false;
	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i == t) {
			found = true;
			break;
		}

		vector<int>::const_iterator beg =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator cit = beg;
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit)
			if (!visited_[*cit]) {
				int const j = *cit;
				visited_[j] = true;
				Q_.push(j);
				int const k = cit - beg;
				prev_edge[j] = vertices_[i].out_edges[k];
				prev_vertex[j] = i;
			}
	}
	if (!found)
		return path;

	while (t != s) {
		path.push_back(prev_edge[t]);
		t = prev_vertex[t];
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

void Graph::addEdge(int s, int t)
{
	vertices_[t].in_vertices.push_back(s);
	vertices_[s].out_vertices.push_back(t);
	vertices_[s].out_edges.push_back(numedges_++);
}

vector<Graph::Vertex> Graph::vertices_;


} // namespace lyx
