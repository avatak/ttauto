// <LICENSE
//   ttauto: a C++ library for building train track automata
//
//   https://github.com/jeanluct/ttauto
//
//   Copyright (C) 2010-2014  Jean-Luc Thiffeault   <jeanluc@math.wisc.edu>
//                            Erwan Lanneau <erwan.lanneau@ujf-grenoble.fr>
//
//   This file is part of ttauto.
//
//   ttauto is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   ttauto is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with ttauto.  If not, see <http://www.gnu.org/licenses/>.
// LICENSE>

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "edge.hpp"
#include "multigon.hpp"
#include "traintracks_util.hpp"

namespace ttauto {

// Attach a new edge to prong p, numbered edge e.
// Prongs and outgoing edges are numbered clockwise.
void multigon::attach_edge(const int p, const int e)
{
  // Make a new edge.
#if __cplusplus > 199711L && !defined(TTAUTO_NO_SHARED_PTR)
  edgep newep(std::make_shared<edge>());
#else
  edgep newep(new edge());
#endif
  attach_edge(newep,p,e);
}

// Attach an edge eg to prong p, numbered edge e.
// Prongs and outgoing edges are numbered clockwise.
void multigon::attach_edge(edgep eg, const int p, const int e)
{
  if (debug)
    {
      std::cerr << "Attaching to prong " << p;
      std::cerr << " as edge " << e << " in multigon::attach_edge.\n";
    }
  /* Change default behaviour: if p and e unspecified, look for the
     first unattached prong/edge or create a new one. Don't use default
     arguments. */
  point_to_edge(eg,p,e);

  // Make sure the edge knows where it's attached.
  eg->point_to_multigon(shared_from_this(),p,e);
}

void multigon::point_to_edge(edgep eg, const int p, const int e)
{
  if (debug && (p < 0 || p >= prongs()))
    {
      std::cerr << "Nonexistent prong " << p;
      std::cerr << " in multigon::point_to_edge.\n";
      std::exit(1);
    }

  // Make sure vector has sufficient capacity.
  if (e >= edges(p)) { egv[p].resize(e+1); }

  if (!prong_edge_is_unattached(p,e))
    {
      std::cerr << "An edge is already attached to prong " << p;
      std::cerr << " edge " << e << " in multigon::point_to_edge.\n";
      std::exit(1);
    }

  egv[p][e] = eg;
}

// Insert a new edge, which unlike attach_edge shifts the existing edges
// out of the way.
void multigon::insert_edge(const int p, const int e)
{
  // Make a new edge.
#if __cplusplus > 199711L && !defined(TTAUTO_NO_SHARED_PTR)
  edgep newep(std::make_shared<edge>());
#else
  edgep newep(new edge());
#endif
  insert_edge(newep,p,e);
}

// Insert an edge eg, which unlike attach_edge shifts the existing edges
// out of the way.
void multigon::insert_edge(edgep eg, const int p, const int e)
{
  // If there is nothing there, can just attach.
  if (e >= edges(p)) return attach_edge(eg,p,e);
  if (prong_edge_is_unattached(p,e)) return attach_edge(eg,p,e);

  // Insert the edge pointer at slot e.
  egv[p].insert(egv[p].begin() + e,eg);

  // Make sure the edge knows where it's attached.
  eg->point_to_multigon(shared_from_this(),p,e);

  // Now update the edge numbers on the edges after e.  The edge
  // pointers and prongs don't need updating.
  for (int ee = e+1; ee < edges(p); ++ee)
    {
      std::cerr << "here1\n";
      int en = Edge(p,ee)->which_ending(shared_from_this());
      Edge(p,ee)->pre[en] = ee;
    }
}

bool multigon::check() const
{
  for (int p = 0; p < k; ++p)
    {
      for (int e = 0; e < edges(p); ++e)
	{
	  if (prong_edge_is_unattached(p,e))
	    {
	      std::cerr << "Unhooked prong edge in multigon::check.\n";
	      std::exit(1);
	    }
	  std::cerr << "here2\n";
	  int en = Edge(p,e)->which_ending(shared_from_this());
	  if (Edge(p,e)->pr[en] != p)
	    {
	      std::cerr << "Inconsistent prong number";
	      std::cerr << " in multigon::check.\n";
	      std::exit(1);
	    }
	  if (Edge(p,e)->pre[en] != e)
	    {
	      std::cerr << "Inconsistent edge number";
	      std::cerr << " in multigon::check.\n";
	      std::exit(1);
	    }
#ifndef TTAUTO_NO_SHARED_PTR
	  if (Edge(p,e).use_count() != 2)
	    {
	      std::cerr << "Bad edge reference counter";
	      std::cerr << " in multigon::check.\n";
	      //std::exit(1);
	    }
#endif
	}
    }
  return true;
}

inline double multigon::weight(const int p, const int e) const
{
  return Edge(p,e)->weight();
}

inline void multigon::weight(const int p, const int e, const double w) const
{
  Edge(p,e)->weight(w);
}

void multigon::cycle_prongs(const int i)
{
  if (debug) std::cerr << "multigon::cycle_prongs\n";
  if (i == 0 || k == 1) return;

  // Use -i to get clockwise cycling.
  std::rotate(egv.begin(),egv.begin()+ttauto::mod(-i,k),egv.end());

  /*update_edge_prong_pointers(this);*/
  update_edge_prong_pointers(std::const_pointer_cast<const multigon>(shared_from_this()));
}

multigon::intVec multigon::edge_sequence(const int p0) const
{
  intVec es(k);

  // uint's are dangerous here, since p0 can be negative.

  for (int p = 0; p < k; ++p)
    {
      // Use a modified mod function that guarantees positive numbers.
      es[p] = edges(ttauto::mod(p + p0,k));
    }

  return es;
}

// For now operator== does not assume that *this or mm is normalised.
// Normalising means that we have to use a consistent train track,
// since edges are checked.
// If the multigons are normalised we can avoid using cyclic_equal.
bool multigon::operator==(const multigon& mm) const
{
  if (k != mm.k || lab != mm.lab) return false;

  intVec es1 = edge_sequence();
  intVec es2 = mm.edge_sequence();

  return cyclic_equal(es1,es2);
}

// operator< assumes that *this and mm are normalised.
bool multigon::operator<(const multigon& mm) const
{
  // Number of prongs is the highest level of inequality.
  if (k != mm.k) return (k < mm.k);

  // Then comes the number of outgoing edges.
  if (edges() != mm.edges()) return (edges() < mm.edges());

  // Then the labels.
  if (lab != mm.lab) return (lab < mm.lab);

  // Then use lexicographical compare for vectors.
  return (edge_sequence() < mm.edge_sequence());
}

multigon::intVec multigon::normalise()
{
  intVec esmax = edge_sequence();
  int pmax = 0;

  for (int p = 1; p < k; ++p)
    {
      // Get an edge sequence offset by -p, which corresponds to
      // cycling the prongs clockwise by p.
      intVec es = edge_sequence(-p);
      if (es > esmax)
	{
	  pmax = p;
	  esmax = es;
	}
    }
  // Cycle so that the prongs are in the 'maximal' position.
  cycle_prongs(pmax);
  return esmax;
}

// Print some information about the multigon.
std::ostream& multigon::print(std::ostream& strm) const
{
  if (punct) strm << "punctured ";

  if (k == 1)
    {
      int ne = edges(0);
      strm << "monogon with label " << label();
      strm << " connected to " << ne << " edge";
      strm << (ne > 1 ? "s" : "") << std::endl;
      return strm;
    }
  else if (k == 2)
    strm << "bi";
  else if (k == 3)
    strm << "tri";
  else if (k == 4)
    strm << "tetra";
  else if (k == 5)
    strm << "penta";
  else if (k == 6)
    strm << "hexa";
  else
    strm << k << "-";
  strm << "gon with label " << label();
  strm << " and edge sequence ";
  for (int p = 0; p < k; ++p)
    {
      strm << edges(p) << " ";
    }
  strm << std::endl;
  return strm;
}

void multigon::erase_edge_pointer(const int p, const int e)
{
  // Erase the edge pointer (the edge is untouched).
  egv[p].erase(egv[p].begin() + e);
  // Now update the edge numbers on the remaining edges.  The edge
  // pointers and prongs don't need updating.
  for (int ee = e; ee < edges(p); ++ee)
    {
      std::cerr << "here3\n";
      int en = Edge(p,ee)->which_ending(shared_from_this());
      Edge(p,ee)->pre[en] = ee;
    }
}

// Cycle through edge pointers, find the end pointing to pm_old, and
// update to point to this.
/*void multigon::update_edge_prong_pointers(const multigon *pm_old)*/
void multigon::update_edge_prong_pointers(const std::shared_ptr<const multigon> pm_old)
{
  std::shared_ptr<multigon> pm_new = shared_from_this();

  for (int p = 0; p < prongs(); ++p)
    {
      for (int e = 0; e < edges(p); ++e)
	{
	  if (Edge(p,e)->attached_to_same_multigon())
	    {
	      // Special case: the two prongs we're trying to swap are
	      // hooked to the same edge.  We've already been here
	      // once, and now both point to pm_old.  So if we just
	      // let the loop carry on it'll see the first ending and
	      // change it back.  So we interrupt the loop and update
	      // the second edge ending.
	      std::shared_ptr<multigon> mg0 = Edge(p,e)->mg[0].lock();
	      if (mg0 == pm_old)
		{
		  // We've already updated mg[0], so we must now
		  // update mg[1].
		  // (On the first pass, we used swap_endings if
		  // necessary to ensure the the substitution occured
		  // on the first ending -- see below.)
		  std::shared_ptr<multigon> mg1 = Edge(p,e)->mg[1].lock();
		  mg1 = pm_new;
		  // For swap it's not strictly necessary to update p
		  // and e, since they haven't changed.  Do it for
		  // completeness, and so cycle_prongs can use this.
		  Edge(p,e)->pr[1] = p;
		  Edge(p,e)->pre[1] = e;
		}
	      else
		{
		  // There's a problem.  Two edges should not point to
		  // the same multigon.
		  std::cerr << "Both ends hooked to same multigon in ";
		  std::cerr << "multigon::update_edge_prong_pointers.\n";
		  std::exit(1);
		}
	    }
	  else
	    {
	      // Find which end points to the old multigon.
	      std::cerr << "here4\n";
	      int en = Edge(p,e)->which_ending(pm_old);
	      Edge(p,e)->mg[en] = pm_new;
	      // For swap it's not strictly necessary to
	      // update p and e, since they haven't changed.
	      // Do it for completeness, and so cycle_prongs
	      // can use this.
	      Edge(p,e)->pr[en] = p;
	      Edge(p,e)->pre[en] = e;
	      if (Edge(p,e)->attached_to_same_multigon() && en == 1)
		{
		  // As a final subtle point (grrr), if the
		  // (transient) duplicate multigon occured on the
		  // second edge ending, swap the endings so the next
		  // pass can assume it occured on the first ending.
		  Edge(p,e)->swap_endings();
		}
	    }
	}
    }
}

//
// Functions and friends
//

// Swap two multigons.
// The tricky bit is updating the edge pointers back to the multigons.
#if 0
void swap(multigon& m1, multigon& m2)
{
  // Save pointers to m1 and m2.
  multigon *pm1 = &m1, *pm2 = &m2;

  // Swap their contents.
  std::swap(m1,m2);

  // Update the edge pointers for m1.
  m1.update_edge_prong_pointers(pm2);
  // Now the train track might momentarilty be in an inconsistent
  // state, if the two multigons share an edge.  But everything will
  // be ok after the next step.

  // Update the edge pointers for m2.
  m2.update_edge_prong_pointers(pm1);
}
#endif

// Swap two multigons.
// The tricky bit is updating the edge pointers back to the multigons.
void swap(std::shared_ptr<multigon> m1, std::shared_ptr<multigon> m2)
{
  std::cerr << "<<<<<<< in swap ============================================\n";

  /*
  const multigon* p1 = m1.get();
  const multigon* p2 = m2.get();
  */

  // Swap their contents.
  std::cerr << "BEFORE:\n";
  std::cerr << m1 << " with " << m1->k << " prongs" << std::endl;
  for (int p = 0; p < m1->prongs(); ++p)
    {
      for (int e = 0; e < m1->edges(p); ++e)
	{
	  std::cerr << "  p=" << p << " e=" << e << " " << m1->Edge(p,e)->mg[0].lock() << " " << m1->Edge(p,e)->mg[1].lock() << std::endl;
	}
    }
  std::cerr << std::endl;
  std::cerr << m2 << " with " << m2->k << " prongs" << std::endl;
  for (int p = 0; p < m2->prongs(); ++p)
    {
      for (int e = 0; e < m2->edges(p); ++e)
	{
	  std::cerr << "  p=" << p << " e=" << e << " " << m2->Edge(p,e)->mg[0].lock() << " " << m2->Edge(p,e)->mg[1].lock() << std::endl;
	}
    }

  std::swap(*m1,*m2);

  for (int p = 0; p < m1->prongs(); ++p)
    {
      for (int e = 0; e < m1->edges(p); ++e)
	{
	  for (int ee = 0; ee < 2; ++e)
	    {
	      std::shared_ptr<multigon> mgee = m1->Edge(p,e)->mg[ee].lock();
	      if (mgee == m1)
		{
		  std::cerr << "  swapping p1 & p2";
		  mgee = m2;
		}
	      if (mgee == m2)
		{
		  std::cerr << "  swapping p1 & p2";
		  mgee = m1;
		}
	    }
	}
    }

  for (int p = 0; p < m2->prongs(); ++p)
    {
      for (int e = 0; e < m2->edges(p); ++e)
	{
	  for (int ee = 0; ee < 2; ++e)
	    {
	      std::shared_ptr<multigon> mgee = m2->Edge(p,e)->mg[ee].lock();
	      if (mgee == m1)
		{
		  std::cerr << "  swapping p1 & p2";
		  mgee = m2;
		}
	      if (mgee == m2)
		{
		  std::cerr << "  swapping p1 & p2";
		  mgee = m1;
		}
	    }
	}
    }




  std::cerr << "\nAFTER:\n";
  std::cerr << m1 << " with " << m1->k << " prongs" << std::endl;
  for (int p = 0; p < m1->prongs(); ++p)
    {
      for (int e = 0; e < m1->edges(p); ++e)
	{
	  std::cerr << "  p=" << p << " e=" << e << " " << m1->Edge(p,e)->mg[0].lock() << " " << m1->Edge(p,e)->mg[1].lock() << std::endl;
	}
    }
  std::cerr << std::endl;
  std::cerr << m2 << " with " << m2->k << " prongs" << std::endl;
  for (int p = 0; p < m2->prongs(); ++p)
    {
      for (int e = 0; e < m2->edges(p); ++e)
	{
	  std::cerr << "  p=" << p << " e=" << e << " " << m2->Edge(p,e)->mg[0].lock() << " " << m2->Edge(p,e)->mg[1].lock() << std::endl;
	}
    }

#if 0
  // Update the edge pointers for m1.
  m1->update_edge_prong_pointers(m2);
  // Now the train track might momentarilty be in an inconsistent
  // state, if the two multigons share an edge.  But everything will
  // be ok after the next step.

  std::cerr << "in swap 3\n";

  // Update the edge pointers for m2.
  m2->update_edge_prong_pointers(m1);

  std::cerr << "   end: " << m1 << " " << m2 << " " << m1->k << " " << m2->k << std::endl;
#endif
  std::cerr << ">>>>>>> in swap ============================================\n";

}

} // namespace ttauto
