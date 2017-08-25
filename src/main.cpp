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
#include <cmath>
extern "C" {
#include "gurobi_c.h"
}

#include "../include/utils.h"
#include "../include/instance.h"
#include "../include/typedef.hpp"
#include "../include/FWChrono.h"

#define LOGS true

using namespace std;

void error_quit(GRBenv**, GRBmodel**);

struct callback_data {
  double  lastiter;
  double  lastnode;
  double *solution;
  FILE   *logfile;
};

int __stdcall mycallback(GRBmodel*, void*, int, void*);

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

	in_graph.show_data();

	// Gurobi variables
	GRBenv* env = NULL;
	GRBmodel* model = NULL;
	int error = 0;
	int* ind = new int[(n * R) + (m * R) + R];
	double* val = new double[(n * R) + (m * R) + R];
	double* obj = new double[(n * R) + (m * R) + R];
	double* lb = new double[(n * R) + (m * R) + R];
	double* ub = new double[(n * R) + (m * R) + R];
	char* vtype = new char[(n * R) + (m * R) + R];
	char** varnames = new char*[(n * R) + (m * R) + R];

	for(int i = 0; i < ((n * R) + (m * R) + R); i++)
		varnames[i] = new char[100];

	char pname[100];
	sprintf(pname, "(%d,%d)-%s.log", k, c, args[1]);

	// Creating environment
	// error = GRBloadenv(&env, pname);
	error = GRBloadenv(&env, NULL);
	if(error) error_quit(&env, &model);

	// Adding variables information in aux vectors
	int counter = 0;
	// x_{vj} variables
	for(int v = 0; v < n; v++)
		for(int j = 0; j < R; j++) {
			ind[counter] = counter;
			obj[counter] = 0.0;
			lb[counter] = 0.0;
			ub[counter] = 1.0;
			vtype[counter] = GRB_BINARY;
			sprintf(varnames[counter], "x_%d_%d", v, j);
			++counter;
		} // counter = n * R

	// y_{uvj} variables
	for(int e = 0; e < m; e++)
		for(int j = 0; j < R; j++) {
			ind[counter] = counter;
			obj[counter] = 0.0;
			lb[counter] = 0.0;
			ub[counter] = 1.0;
			vtype[counter] = GRB_BINARY;
			sprintf(varnames[counter], "y_%d_%d", e, j);
			++counter;
		} // counter = n * R + m * R

	// w_j variables
	for(int j = 0; j < R; j++) {
		ind[counter] = counter;
		obj[counter] = 1.0;
		lb[counter] = 0.0;
		ub[counter] = 1.0;
		vtype[counter] = GRB_BINARY;
		sprintf(varnames[counter], "w_%d", j);
		++counter;
	} // counter = (n * R) + (m * R) + R

	error = GRBnewmodel(env, &model, pname, counter, obj, lb, ub, vtype, varnames);
	if(error) error_quit(&env, &model);

	char name[100];

	// Constraints (2)
	for(int v = 0; v < n; v++) {
		for(int j = 0; j < R; j++) {
			ind[j] = v * R + j;
			val[j] = 1.0;
		}
		sprintf(name, "cons2_%d", v);
		error = GRBaddconstr(model, R, ind, val, GRB_EQUAL, k, name);
    if (error) error_quit(&env, &model);
	}

	// Constraints (3)
	for(int e = 0; e < m; e++) {
		for(int j = 0; j < R; j++) {
			ind[j] = (n * R) + e * R + j;
			val[j] = 1.0;
		}
		sprintf(name, "cons3_%d", e);
		error = GRBaddconstr(model, R, ind, val, GRB_LESS_EQUAL, c, name);
    if (error) error_quit(&env, &model);
	}

	// Constraints (4)
	for(int e = 0; e < m; e++) {
		for(int j = 0; j < R; j++) {
			ind[0] = edges[e].first * R + j;
			ind[1] = edges[e].second * R + j;
			val[0] = val[1] = 1.0;
			ind[2] = (n * R) + e * R + j;
			val[2] = -1.0;
			sprintf(name, "cons4_%d_%d", e, j);
			error = GRBaddconstr(model, 3, ind, val, GRB_LESS_EQUAL, 1, name);
	    if (error) error_quit(&env, &model);
		}
	}

	// Constraints (5)
	for(int v = 0; v < n; v++) {
		for(int j = 0; j < R; j++) {
			ind[0] = v * R + j;
			val[0] = 1.0;
			ind[1] = (n * R) + (m * R) + j;
			val[1] = -1.0;
			sprintf(name, "cons5_%d_%d", v, j);
			error = GRBaddconstr(model, 2, ind, val, GRB_LESS_EQUAL, 0, name);
	    if (error) error_quit(&env, &model);
		}
	}

	// Constraints (6)
	for(int j = 0; j < R - 1; j++) {
		ind[0] = (n * R) + (m * R) + j;
		val[0] = 1.0;
		ind[1] = (n * R) + (m * R) + j + 1;
		val[1] = -1.0;
		sprintf(name, "cons6_%d", j);
		error = GRBaddconstr(model, 2, ind, val, GRB_LESS_EQUAL, 0, name);
    if (error) error_quit(&env, &model);
	}

	// Model solution and configuration of attrs
	GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MINIMIZE);

	error = GRBsetintparam(env, GRB_INT_PAR_THREADS, 1);
	if (error) error_quit(&env, &model);
	GRBsetintparam(env, GRB_INT_PAR_AGGREGATE, 0);
	GRBsetintparam(env, GRB_INT_PAR_PRESOLVE, 0);
	GRBsetintparam(env, GRB_INT_PAR_CUTS, 0);
	GRBsetdblparam(env, GRB_DBL_PAR_HEURISTICS, 0);
	GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, 3600.0);

	// Callback settings
	struct callback_data mydata;
  mydata.lastiter = -GRB_INFINITY;
  mydata.lastnode = -GRB_INFINITY;
  mydata.solution = (double*) malloc(((n * R) + (m * R) + R) * sizeof(double));;
  mydata.logfile  = fopen("cb.log", "w");
	error = GRBsetcallbackfunc(model, mycallback, (void *) &mydata);
  if (error) error_quit(&env, &model);

	GRBwrite(model, "test.lp");
	error = GRBoptimize(model);
	if (error) error_quit(&env, &model);

	int optimstatus;
	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
  if (error) error_quit(&env, &model);

	double objval;
  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (error) error_quit(&env, &model);

	printf("\nOptimization complete\n");
  if (optimstatus == GRB_OPTIMAL) {
    printf("Optimal objective: %.4lf\n", objval);
  } else if (optimstatus == GRB_INF_OR_UNBD) {
    printf("Model is infeasible or unbounded\n");
  } else {
    printf("Optimization was stopped early\n");
  }

	GRBfreemodel(model);
  GRBfreeenv(env);

	return 0;
}

void error_quit(GRBenv** env, GRBmodel** model) {
  printf("ERROR: %s\n", GRBgeterrormsg(*env));
	GRBfreemodel(*model);
	GRBfreeenv(*env);
  exit(1);
}

int __stdcall mycallback(GRBmodel *model, void *cbdata, int where, void *usrdata) {
 struct callback_data *mydata = (struct callback_data *) usrdata;

 if (where == GRB_CB_POLLING) {
	 /* Ignore polling callback */
 } else if (where == GRB_CB_PRESOLVE) {
	 /* Presolve callback */
 } else if (where == GRB_CB_SIMPLEX) {
	 /* Simplex callback */
 } else if (where == GRB_CB_MIP) {
	 /* General MIP callback */
	 double nodecnt, objbst, objbnd, actnodes, itcnt;
	 int    solcnt, cutcnt;
	 GRBcbget(cbdata, where, GRB_CB_MIP_NODCNT, &nodecnt);
	 GRBcbget(cbdata, where, GRB_CB_MIP_OBJBST, &objbst);
	 GRBcbget(cbdata, where, GRB_CB_MIP_OBJBND, &objbnd);
	 GRBcbget(cbdata, where, GRB_CB_MIP_SOLCNT, &solcnt);
	 if (nodecnt - mydata->lastnode >= 100) {
		 mydata->lastnode = nodecnt;
		 GRBcbget(cbdata, where, GRB_CB_MIP_NODLFT, &actnodes);
		 GRBcbget(cbdata, where, GRB_CB_MIP_ITRCNT, &itcnt);
		 GRBcbget(cbdata, where, GRB_CB_MIP_CUTCNT, &cutcnt);
		 printf("%7.0f %7.0f %8.0f %13.6e %13.6e %7d %7d\n",
						nodecnt, actnodes, itcnt, objbst, objbnd, solcnt, cutcnt);
	 }
	 if (fabs(objbst - objbnd) < 0.1 * (1.0 + fabs(objbst))) {
		 printf("Stop early - 10%% gap achieved\n");
		 GRBterminate(model);
	 }
	 if (nodecnt >= 10000 && solcnt) {
		 printf("Stop early - 10000 nodes explored\n");
		 GRBterminate(model);
	 }
 } else if (where == GRB_CB_MIPSOL) {
	 /* MIP solution callback */
	 double nodecnt, obj;
	 int    solcnt;
	 GRBcbget(cbdata, where, GRB_CB_MIPSOL_NODCNT, &nodecnt);
	 GRBcbget(cbdata, where, GRB_CB_MIPSOL_OBJ, &obj);
	 GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOLCNT, &solcnt);
	 GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, mydata->solution);
	 printf("**** New solution at node %.0f, obj %g, sol %d, x[0] = %.2f ****\n",
					nodecnt, obj, solcnt, mydata->solution[0]);
 } else if (where == GRB_CB_MIPNODE) {
	 int status;
	 /* MIP node callback */
	 printf("**** New node ****\n");
	 GRBcbget(cbdata, where, GRB_CB_MIPNODE_STATUS, &status);
	 if (status == GRB_OPTIMAL) {
		 GRBcbget(cbdata, where, GRB_CB_MIPNODE_REL, mydata->solution);
		 GRBcbsolution(cbdata, mydata->solution, NULL);
	 }
 } else if (where == GRB_CB_MESSAGE) {
	 /* Message callback */
 }
 return 0;
}
