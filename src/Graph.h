// -*- C++ -*-
/**
 * \file Graph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur (original code)
 * \author Richard Kimberly Heck (re-implementation)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GRAPH_H
#define GRAPH_H


#include <list>
#include <queue>
#include <set>
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
	EdgePath const getReachableTo(int to, bool clear_visited);
	/// \return a vector of the reachable vertices, avoiding all "excludes"
	EdgePath const getReachable(int from, bool only_viewable,
		bool clear_visited, std::set<int> excludes = std::set<int>());
	/// can "from" be reached from "to"?
	bool isReachable(int from, int to);
	/// find a path from "from" to "to". always returns one of the
	/// shortest such paths.
	EdgePath const getPath(int from, int to);
	/// called repeatedly to build the graph
	void addEdge(int from, int to);
	/// reset the internal data structures
	void init(int size);

private:
	///
	bool bfs_init(int, bool clear_visited, std::queue<int> & Q);
	/// these represent the arrows connecting the nodes of the graph.
	/// this is the basic representation of the graph: as a bunch of
	/// arrows.
	struct Arrow {
		///
		Arrow(int f, int t, int i):
			from(f), to(t), id(i) {}
		/// the vertex at the tail of the arrow
		int from;
		/// the vertex at the head
		int to;
		/// an id for this arrow, e.g., for use in describing paths
		/// through the graph
		int id;
	};
	/// a container for the arrows
	/// we use a list because we want pointers to the arrows,
	/// and a vector might invalidate them
	typedef std::list<Arrow> Arrows;
	Arrows arrows_;
	/// Represents a vertex of the graph. Note that we could recover
	/// the in_arrows and out_arrows from the Arrows, so these are in
	/// effect a kind of cache.
	struct Vertex {
		/// arrows that point at this one
		std::vector<Arrow *> in_arrows;
		/// arrows out from here
		std::vector<Arrow *> out_arrows;
		/// used in the search routines
		bool visited;
	};
	/// a container for the vertices
	/// the index into the vector functions as the identifier by which
	/// these are referenced in the Arrow struct
	/// the code making use of the Graph must keep track of the relation
	/// between these indices and the objects they represent. (in the case
	/// of Format, this is easy, since the Format objects already have ints
	/// as identifiers.)
	std::vector<Vertex> vertices_;

	/// a counter that we use to assign id's to the arrows
	/// FIXME This technique assumes a correspondence between the
	/// ids of the arrows and ids associated with Converters that
	/// seems kind of fragile. Perhaps a better solution would be
	/// to pass the ids as we create the arrows.
	int numedges_;
};



} // namespace lyx

#endif //GRAPH_H
