#include <iostream>
#include <ext/hash_map>
#include <jlt/mathmatrix.hpp>
#include "traintrack.hpp"
#include "ttfoldgraph.hpp"
#include "folding_path.hpp"
#include "pAclass.hpp"

using namespace traintracks;

int main()
{
  using std::cout;
  using std::endl;
  typedef jlt::mathmatrix<int> Mat;

  typedef traintrack TrTr;
  const int n = 6;
  traintrack tt(n,3);
  // traintrack tt(n);

  cout << "Train track has " << tt.punctures() << " punctures and ";
  cout << tt.edges() << " edges\n";

  ttfoldgraph<TrTr> ttg(tt);

  // ttg.print();

  cout << "TT folding graph has " << ttg.vertices() << " vertices\n";

  folding_path<TrTr> p(ttg,1,4);

  int ep = -1;
  do
    {
      cout << "p=" << p << " [" << p.vertices() << "] ";
      if (p.closed())
	cout << "p*p=" << p*p << " [" << (p*p).vertices() << "]";
      cout << " endpath = " << p.subpath(ep)
	   << " ["  << p.subpath(ep).vertices() << "]";
      cout << endl;
    }
  while (++p);

  // p1 = 1:2121, p2 = 4:1212.

  folding_path<TrTr> p1(ttg,1);
  p1.push_back(2);
  p1.push_back(1);
  p1.push_back(2);
  p1.push_back(1);

  cout << "p1 = " << p1 << "  [" << p1.vertices() << "]" << endl;

  folding_path<TrTr> p2(p1);
  p2.cycle_path(-1);
  cout << "p2 = " << p2 << "  [" << p2.vertices() << "]" << endl;

  if (p1 == p2)
    cout << "Paths are equal!\n";
  else
    cout << "Paths are unequal...\n";

  typedef folding_path<TrTr>::hash path_hash;
  typedef __gnu_cxx::hash_map<folding_path<TrTr>,int,path_hash> pathlist;

  pathlist pl;

  path_hash f;
  cout << "Hash keys: f(p1)=" << f(p1) << " f(p2)=" << f(p2);
  if (pl.key_eq()(p1,p2))
    cout << " are equal!\n";
  else
    cout << " are not equal...\n";

  // Add some elements to the hash map.
  pl[p1] = 1;
  cout << "Add element with key p1 to hash map: size=" << pl.size() << endl;
  // Add again: doesn't increase size, since we already have it.
  pl[p2] = 2;
  cout << "Add element with key p2 to hash map: size=" << pl.size() << endl;

  cout << "Cycle p2 back by 1: ";
  p2.cycle_path(1);
  cout << "p2 = " << p2 << "  [" << p2.vertices() << "]" << endl;

  return 0;
}