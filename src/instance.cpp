//============================================================================
// Name        : instance.cpp
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Instance data class implementation
//============================================================================

#include "../include/instance.h"

instance::instance() { }

instance::instance( int _n, int _m, int _k, int _c, int _R ) : n(_n), m(_m), k(_k), c(_c), R(_R) {
	edges = vector< edge >(m);
}

instance::~instance() { }

void instance::set_n( int _n ) { this->n = _n; }

void instance::set_m( int _m ) { this->m = _m; }

void instance::set_k( int _k ) { this->k = _k; }

void instance::set_c( int _c ) { this->c = _c; }

void instance::set_R( int _R ) { this->R = _R; }

void instance::set_edges( vector< edge >& _edges) { this->edges = _edges; }

int instance::get_n() { return this->n; }

int instance::get_m() { return this->m; }

int instance::get_k() { return this->k; }

int instance::get_c() { return this->c; }

int instance::get_R() { return this->R; }

const vector< edge >& instance::get_edges() const { return this->edges; }

void instance::read_data() {
	scanf("%d %d %d %d %d", &n, &m, &k, &c, &R);

	edges = vector< edge >(m);
	for(int e = 0; e < m; e++) {
		int v1, v2;
		scanf("%d %d", &v1, &v2);
		edges[e] = make_pair(v1, v2);
	}
}

void instance::read_from_file( char* filename ) {
	ifstream _file;
  _file.open(filename, std::ifstream::in);

  if(_file.is_open()) {
		_file >> n >> m; //>> k >> c >> R;

		edges = vector< edge >(m);
		for(int e = 0; e < m; e++) {
			int v1, v2;
			_file >> v1 >> v2;
			edges[e] = make_pair(v1, v2);
		}
	}
}

void instance::show_data() {
	printf("Number of vertices & edges: %d %d\n", n, m);
	printf("Number (k,c) = (%d, %d) & |R| = %d\n", k, c, R);
	printf("Edges list:\n");
	for(int e = 0; e < m; e++)
			printf("%d <-> %d\n", edges[e].first, edges[e].second);
}
