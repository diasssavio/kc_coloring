//============================================================================
// Name        : main.cpp
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Main program file
//============================================================================

#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <ilcplex/ilocplex.h>

#include "../include/instance.h"
#include "../include/model.h"
#include "../include/callbacks.h"
#include "../include/typedef.hpp"
#include "../include/FWChrono.h"

#define LOGS true

using namespace std;

template<typename T>
T string_to(const string& s){
	istringstream i(s);
	T x;
	if (!(i >> x)) return 0;
	return x;
}

template<typename T>
string to_string2(const T& t){
  stringstream ss;
  if(!(ss << t)) return "";
  return ss.str();
}

ILOSTLBEGIN

int main(int argc, char* args[]){
	instance in_graph;
  int n, m, k, c, R;
	vector< edge > edges;

  if(argc >= 2) {
  	in_graph.read_from_file(args[1]);
  	n = in_graph.get_n();
		m = in_graph.get_m();
		if (argc >= 3) {
			k = string_to< int >(args[2]);
			c = string_to< int >(args[3]);
			R = string_to< int >(args[4]);
			in_graph.set_k(k);
			in_graph.set_c(c);
			in_graph.set_R(R);
		}
		edges = in_graph.get_edges();
  }

	// Initializing cplex environment
	IloEnv env;

  ofstream _file;
  IloNum linear_obj = 0.0;
  double linear_time = 0.0;
  FWChrono timer;
  timer.start();
	try {
		model mod(env, in_graph);

		_file.open("cutset.txt", ios::trunc);

		IloCplex cplex(mod);
		cplex.setParam(IloCplex::Threads, 1);
		// cplex.setParam(IloCplex::NodeLim, 0);
		cplex.setParam(IloCplex::Param::Preprocessing::Aggregator, 0);
		cplex.setParam(IloCplex::Param::Preprocessing::Presolve, 0);
		cplex.setParam(IloCplex::PreInd, 0);
		cplex.setParam(IloCplex::CutsFactor, 1);
		cplex.setParam(IloCplex::HeurFreq, -1);
    cplex.setParam(IloCplex::Param::TimeLimit, 3600);
		cplex.exportModel("test.lp");

    // cplex.use(IloCplex::Callback(new (env) hao_cutsetcallback(env, mod.x, in_graph, _file)));
    // cplex.use(IloCplex::Callback(new (env) hao_cutsetcallback2(env, mod.x, in_graph, _file)));
    cplex.use(IloCplex::Callback(new (env) mipinfo_callback(env, timer, &linear_obj, &linear_time)));

#if LOGS != true
			cplex.setOut(env.getNullStream());
#endif

		cplex.solve();
    timer.stop();
    _file.close();

#if LOGS == true
		// Printing solution information
		printf("Objective value = %.2lf\n", cplex.getObjValue());
		printf("Number of integer solutions: %ld\n", cplex.getSolnPoolNsolns());
    printf("Number of nodes: %ld\n", cplex.getNnodes());
    printf("Linear Relaxation value = %.3lf (%.3lf)\n", linear_obj, linear_time);

		// Printing solution
		for(int v = 0; v < n; v++) {
			printf("Vertex #%d: ", v);
			for(int j = 0; j < R; j++)
				if(cplex.getValue(mod.x[v][j]) >= EPSILON)
					printf("%d ", j + 1);
			printf("\n");
		}

		for(int e = 0; e < m; e++) {
			printf("Edge #%d (%d <-> %d): ", e, edges[e].first, edges[e].second);
			for(int j = 0; j < R; j++)
				if(cplex.getValue(mod.y[e][j]) >= EPSILON)
					printf("%d ", j + 1);
			printf("\n");
		}
#endif

    // Save the resulting data in a .csv file
		_file.open("results.csv", ios::app);
		if(_file.is_open()){
			_file.precision(3);
			_file << fixed << cplex.getBestObjValue() << ", ," << timer.getStopTime()	<< "," << linear_obj << "," << linear_time << ","
            << cplex.getSolnPoolNsolns() << "," << cplex.getNnodes() << endl;
			_file.close();
		}
	} catch(IloException& e) {
		cerr << "CONCERT EXCEPTION -- " << e << endl;
	}
	// Closing the environment
	env.end();

	return 0;
}
