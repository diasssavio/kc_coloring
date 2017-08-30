//============================================================================
// Name        : utils.h
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Instance data class interface
//============================================================================

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <utility>
extern "C" {
#include "gurobi_c.h"
}

#include "typedef.hpp"
#include "instance.h"
#include "FWChrono.h"

#define EPSILON 1e-5

using namespace std;

vector< int > get_adjacency(int, vector<edge>&);
bool is_adjacent(int, int, vector<edge>&);
vector< int > get_edges_from_clique(vector< int >&, vector<edge>&);

bool comp(pair< int, double >, pair< int, double >);

typedef struct callback_data {
  unsigned   numvars;
  unsigned   cuts_applied;
  double     linear_rel;
  double     cut_rel;
  double     linear_time;
  bool       flag;
  double     *solution;
  instance   *in_graph;
  FILE       *logfile;
  FWChrono   *timer;
} c_data;

int __stdcall mycallback(GRBmodel*, void*, int, void*);

#endif /* UTILS_H_ */
