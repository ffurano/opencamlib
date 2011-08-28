/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VODI_G_HPP
#define VODI_G_HPP

#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "point.hpp"
#include "halfedgediagram.hpp"
#include "voronoivertex.hpp"

// this file contains typedefs used by voronoidiagram.h
namespace ocl {

// notes from the Okabe-Boots-Sugihara book, page 171->:
/* 
 * Distance-function.
 * R1 - region of endpoint pi1
 * R2 - region of endpoint pi2
 * R3 - region of line-segment Li
 *               dist(p,pi1) if  p in R1
 * dist(p,Li) =  dist(p,pi2) if  p in R2
 *               dist(p,Li)  if p in R3
 * 
 * dist(p,Li) = distance from p to L, along perpendicular line to L
 * 
 * = norm(  (x-xi1)   -  dotprod( (x-xi1), (xi2-xi1) ) / ( norm_sq(xi2-xi1) ) * (xi2,xi1) )
 * 
 * 
 * 
 * Vertex - LineSegment
 * Bisectors:
 *  B1: point-point: line
 *  B2: point-line: parabola
 *  B3: line-line: line
 * 
 *  Voronoi Edges:
 *  E1: point pi - point pj. straight line bisecting pi-pj
 *  E2: edge generated by line-segment L's endpoint pi. perpendicular to L, passing through pi
 *  E3: point pi - segment Lj. dist(E3, p) == dist(E3,Lj). Parabolic arc
 *  E4: line Li - Line Lj. straight line bisector
 *  (G): generator segment edge
 * 
 *  Voronoi vertices (see p177 of Okabe book):
 *  V1: generators(pi, pj, pk). edges(E1, E1, E1)
 *  V2: generators(pi, Lj, pj1) point, segment, segment's endpoint. edges(E1, E2, E3)   E1 and E3 are tangent at V2
 *  V3: generators(Li, pj, pk) edges(E1, E3, E3)   E3-edges have common directrix(Li)
 *  V4: generators(Li, Lj, pi1)  edges(E2, E3, E4)  E3-E4 tangent at V4
 *  V5: generators(pi, Lj, Lk) edges (E3, E3, E4)
 *  V6: generators(Li, Lj, Lk) edges(E4, E4, E4)
 * 
 * 
 * bisector formulas
 * x = x1 - x2 - x3*t +/- x4 * sqrt( square(x5+x6*t) - square(x7+x8*t) )
 * (same formula for y-coordinate)
 * line (line/line)
 * parabola (circle/line)
 * hyperbola (circle/circle)
 * ellipse (circle/circle)
 * 
 * line: a1*x + b1*y + c + k*t = 0  (t is amount of offset) k=+1 offset left of line, k=-1 right of line
 * with a*a + b*b = 1
 * 
 * circle: square(x-xc) + square(y-yc) = square(r+k*t)  k=+1 for enlarging circle, k=-1 shrinking
 */
 

// use traits-class here so that EdgePros can store data of type HEEdge
// typedef of the VD-graph follows below. 
typedef boost::adjacency_list_traits<boost::listS, 
                                     boost::listS, 
                                     boost::bidirectionalS, 
                                     boost::listS >::edge_descriptor HEEdge;
typedef unsigned int HEFace;    
                        

/// properties of an edge in the voronoi diagram
/// each edge stores a pointer to the next HEEdge 
/// and the HEFace to which this HEEdge belongs
struct EdgeProps {
    EdgeProps() {}
    EdgeProps(HEEdge n, HEFace f){
        next = n;
        face = f;
    }
    /// create edge with given next, twin, and face
    EdgeProps(HEEdge n, HEEdge t, HEFace f){
        next = n;
        twin = t;
        face = f;
    }
    /// the next edge, counterclockwise, from this edge
    HEEdge next; 
    /// the twin edge
    HEEdge twin;
    /// the face to which this edge belongs
    HEFace face; 
};

/// types of faces in the voronoi diagram
enum VoronoiFaceStatus {INCIDENT, NONINCIDENT};

/// properties of a face in the voronoi diagram
/// each face stores one edge on the boundary of the face
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( HEEdge e , Point gen, VoronoiFaceStatus t) {
        edge = e;
        generator = gen;
        status = t;
    }
    /// operator for sorting faces
    bool operator<(const FaceProps& f) const {return (this->idx<f.idx);}
    /// face index
    HEFace idx;
    /// one edge that bounds this face
    HEEdge edge;
    /// the generator for this face
    Point generator;
    VoronoiGenerator* gen;
    /// face status (either incident or nonincident)
    VoronoiFaceStatus status;
};



// the type of graph with which we construct the voronoi-diagram
typedef HEDIGraph<     boost::listS,             // out-edges stored in a std::list
                       boost::listS,             // vertex set stored here
                       boost::bidirectionalS,    // bidirectional graph.
                       VoronoiVertex,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS              // edge storage
                       > HEGraph;
// NOTE: if these listS etc. arguments ever change, they must be updated
// above where we do: adjacency_list_traits

typedef boost::graph_traits< HEGraph >::vertex_descriptor  HEVertex;
typedef boost::graph_traits< HEGraph >::vertex_iterator    HEVertexItr;
typedef boost::graph_traits< HEGraph >::edge_iterator      HEEdgeItr;
typedef boost::graph_traits< HEGraph >::out_edge_iterator  HEOutEdgeItr;
typedef boost::graph_traits< HEGraph >::adjacency_iterator HEAdjacencyItr;
typedef boost::graph_traits< HEGraph >::vertices_size_type HEVertexSize;



// these containers are used instead of iterators when accessing
// adjacent vertices, edges, faces.
// FIXME: it may be faster to rewrite the code so it uses iterators, as does the BGL.
typedef std::vector<HEVertex> VertexVector;
typedef std::vector<HEFace> FaceVector;
typedef std::vector<HEEdge> EdgeVector;  


} // end ocl namespace
#endif


/* notes from okabe-boots-sugihara, page 179
 * on point-line-arc diagrams
 * 
 * distance function (to arc Li with center xci and radius ri):
 * dist(p,Li) = norm(  norm(x-xci) -ri )  when in R3
 * 
 * edge types E1-E4 given above for point-line diagrams.
 * new edge types are
 * E5: generators (ARCLi, pj)  pj is inside cricle containing Li centered at pci.
 * edge is at dist(p,pj) = dist(p,pci) i.e elliptic arc whose foci are pci and pj
 * E6: gen(arcLi, pj) pj is outside circle Li centered at pci. 
 * edge is at   dist(p,pci) - dist(p,pj) = constant, i.e. hyperbola whose foci pci and pj
 * E7: gen( arcLi, endpoint pi1). straight line radiating from pci through pi1
 * E8: gen(arcLi, lineLj)  line Lj intersects arcLi-circle
 * dist(p, Li(j?)) + dist(p, pci) = constant, i.e. parabolic arc with focus pci and directrix parallell to Lj at distance ri
 * E9: gen(arcLi, arcLj) circle containing Li is contained in circle containing Lj
 * edge is elliptic arc whose foci are pci and pcj
 * E10: gen(arcLi, arcLj) circle containing Li centered at pci intersects circle containing Lj centered at pcj,
 * OR the former circle is outside the later circle.
 * edge is branch of hyperbolic curve whose foci are pci and pcj
 * 
 * 
 * Similar new list of vertex-types?
 * 
*/
