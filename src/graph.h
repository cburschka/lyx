// -*- C++ -*-
/**
 * \file graph.h
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


class Graph {
public:
	Graph() : numedges_(0) {};
	///
	typedef std::vector<int> EdgePath;
	///
	std::vector<int> const
	getReachableTo(int, bool clear_visited);
	///
	std::vector<int> const
	getReachable(int, bool only_viewable,
		     bool clear_visited);
	///
	bool isReachable(int, int);
	///
	EdgePath const getPath(int, int);
	///
	void addEdge(int s, int t);
	///
	void init(int size);

private:
	///
	int bfs_init(int, bool clear_visited = true);

	///
	class Vertex {
	public:
		std::vector<int> in_vertices;
		std::vector<int> out_vertices;
		std::vector<int> out_edges;
	};
	///
	static
	std::vector<Vertex> vertices_;
	///
	std::vector<bool> visited_;
	///
	std::queue<int> Q_;

	int numedges_;

};


#endif //GRAPH_H
