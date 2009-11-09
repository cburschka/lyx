// -*- C++ -*-
/**
 * \file Graph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <queue>
#include <vector>


namespace lyx {


/// Represents a directed graph, possibly with multiple edges
/// connecting the vertices.
class Graph {
public:
	Graph() : numedges_(0) {}
	///
	typedef std::vector<int> EdgePath;
	/// \return a vector of the vertices from which "to" can be reached
	std::vector<int> const getReachableTo(int to, bool clear_visited);
	/// \return a vector of the vertices that can be reached from "from"
	std::vector<int> const
		getReachable(int from, bool only_viewable, bool clear_visited);
	/// Can "from" be reached from "to"?
	bool isReachable(int from, int to);
	/// Find a path from "from" to "to".
	EdgePath const getPath(int from, int to);
	/// Called repeatedly to build the graph.
	void addEdge(int from, int to);
	/// Reset the internal data structures.
	void init(int size);

private:
	///
	bool bfs_init(int, bool clear_visited = true);

	///
	struct OutEdge {
		OutEdge(int v, int e): vertex(v), edge(e) {}
		int vertex;
		int edge;
	};
	///
	struct Vertex {
		/// vertices that point at this one
		std::vector<int> in_vertices;
		/// paths out from here
		std::vector<OutEdge> out_arrows;
		///
		bool visited;
	};
	///
	std::vector<Vertex> vertices_;
	///
	std::queue<int> Q_;

	int numedges_;

};



} // namespace lyx

#endif //GRAPH_H
