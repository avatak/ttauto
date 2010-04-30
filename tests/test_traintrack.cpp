#include <iostream>
#include "edge.hpp"
#include "multigon.hpp"
#include "traintrack.hpp"

using namespace traintracks;

int main()
{
  using std::cout;
  using std::endl;

  multigon mm1(3);	// A trigon.
  multigon mm2(1);	// A monogon.

  mm1.attach_edge(0,0);		// Attach a new edge to prong 0, edge 0.
  cout << "Reference count = " << mm1.Edge(0,0).use_count() << endl;
  mm2.attach_edge(mm1.Edge(0,0),0,0);	// Attach the same edge to mm2.
  cout << "Reference count = " << mm1.Edge(0,0).use_count() << endl;

  mm1.attach_edge(0,1);		// Attach another edge a prong 0.
  mm1.attach_edge(0,2);		// ...and another.
  mm1.insert_edge(0,1);		// Insert an edge at 1, moving an edge
				// out of the way.

  cout << "\nMultigons mm1 and mm2:\n";
  mm1.print();
  mm2.print();

  // Build a train track with 9 monogons, a trigons, a tetragon,
  // and a pentagon.
  const int N = 9;
  jlt::vector<int> Kv(3);
  Kv[0] = 4;
  Kv[1] = 5;
  Kv[2] = 3;
  traintrack tt(N,Kv);
  tt.set_label(0,8); // Label a monogon.
  cout << "\nBig train track:\n";
  tt.print();
  cout << "\nCoding:\n";
  tt.print_coding() << endl;

  traintrack tt2 = tt;	  // Copy the train track.
  tt2.check();
  tt2.print_coding() << endl;

  // The equality (isotopy) operator.
  if (tt == tt2) cout << "Tracks match!\n"; else cout << "Don't match!\n";

  // Make a train track from a coding.
  traintrack tt3(tt.coding());
  if (tt == tt3) cout << "Tracks match!\n"; else cout << "Don't match!\n";

  jlt::vector<double> w(tt.edges());
  for (int i = 0; i < tt.edges(); ++i) w[i] = i+1;
  tt.weights(w.begin());
  cout << "\nWeights: " << tt.weights() << endl;

  cout << "\nFold clockwise at cusp 0:\n";
  tt.fold(0);
  tt.print_coding() << endl;

  cout << "Weights: " << tt.weights() << endl << endl;

  // Built a list of representatives of all allowable train tracks
  // with 8 punctures (monogons).
  jlt::vector<traintrack> ttv = ttbuild_list(8);
}