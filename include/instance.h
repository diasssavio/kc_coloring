//============================================================================
// Name        : instance.h
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Instance data class interface
//============================================================================

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <cstdio>
#include <vector>
#include <utility>
#include <fstream>
#include "typedef.hpp"

using namespace std;

class instance {
private:
	// Primary parameter
	int n; // Number of nodes on the instance
	int m; // Number of edges on the instance
	int k; // Number of colors to assign to each node
	int c; // Number max of colors each adjacent vertex may share
	int R;

	// General data
	vector < edge > edges; // List of edges in the graph

public:
	// Constructors & Destructors
	instance();
	instance( int, int, int, int, int );
	virtual ~instance();

	// Setters
	void set_n( int );
	void set_m( int );
	void set_k( int );
	void set_c( int );
	void set_R( int );
	void set_edges( vector < edge >& );

	// Getters
	int get_n();
	int get_m();
	int get_k();
	int get_c();
	int get_R();
	const vector < edge >& get_edges() const;

	// Useful Methods
	void read_data();
	void read_from_file( char* );
	void show_data();
};

#endif /* INSTANCE_H_ */
