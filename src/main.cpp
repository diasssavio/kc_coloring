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
#include <cstring>
#include <ctime>
#include <cmath>
#include <cfloat>

extern "C" {
#include "gurobi_c.h"
}

#include "../include/utils.h"
#include "../include/instance.h"
#include "../include/typedef.hpp"
#include "../include/FWChrono.h"

using namespace std;

void error_quit(GRBenv**, GRBmodel**);

int main(int argc, char* args[]){
	instance in_graph;
  int n, m, k, c, R;
	vector< edge > edges;

  FWChrono timer;
  timer.start();

  // Reading instance data from command line argument
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
  int option = 0;
  if(argc >= 6) // Lagrangian Relaxation option (constraints) {2, 3}
    option = string_to< int >(args[5]);

#if LOGS == true
	// in_graph.show_data();
#endif

	// Gurobi variables
	GRBenv* env = NULL;
	GRBmodel* model = NULL;
	int error = 0;
  unsigned numvars = (n * R) + (m * R) + R;
	int* ind = new int[numvars];
	double* val = new double[numvars];
	double* obj = new double[numvars];
	double* lb = new double[numvars];
	double* ub = new double[numvars];
	char* vtype = new char[numvars];
	char** varnames = new char*[numvars];

	for(int i = 0; i < numvars; i++)
		varnames[i] = new char[100];

	char pname[100];
	sprintf(pname, "(%d,%d)-%s.log", k, c, args[1]);

	// Creating environment
	// error = GRBloadenv(&env, pname);
	error = GRBloadenv(&env, "");
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
  if(error) error_quit(&env, &model);

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
  if(option != 2) // Checking whether it is a Lagrangian model
  	for(int v = 0; v < n; v++) {
  		for(int j = 0; j < R; j++) {
  			ind[j] = v * R + j;
  			val[j] = 1.0;
  		}
  		sprintf(name, "cons2_%d", v);
  		error = GRBaddconstr(model, R, ind, val, '=', k, name);
      if (error) error_quit(&env, &model);
  	}

	// Constraints (3)
  if(option != 3) // Checking whether it is a Lagrangian model
  	for(int e = 0; e < m; e++) {
  		for(int j = 0; j < R; j++) {
  			ind[j] = (n * R) + e * R + j;
  			val[j] = 1.0;
  		}
  		sprintf(name, "cons3_%d", e);
  		error = GRBaddconstr(model, R, ind, val, '<', c, name);
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
			error = GRBaddconstr(model, 3, ind, val, '<', 1, name);
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
			error = GRBaddconstr(model, 2, ind, val, '<', 0, name);
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
		error = GRBaddconstr(model, 2, ind, val, '>', 0, name);
    if (error) error_quit(&env, &model);
	}

	// Model solution and configuration of attrs
	GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MINIMIZE);

#if LOGS != true
	GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_OUTPUTFLAG, 0);
#endif

	GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_THREADS, 1);
	GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_AGGREGATE, 0);
	GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_PRESOLVE, 0);
  GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_PRECRUSH, 1);
	GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_CUTS, 0);
	GRBsetdblparam(GRBgetenv(model), GRB_DBL_PAR_HEURISTICS, 0);
	GRBsetdblparam(GRBgetenv(model), GRB_DBL_PAR_TIMELIMIT, 3600.0);

  if(!option) { // i.e. Branch-and-Cut
  	// Callback settings
  	c_data mydata;
    mydata.numvars  = numvars;
    mydata.cuts_applied  = 0;
    mydata.flag     = true;
    mydata.solution = (double*) malloc(numvars * sizeof(double));
    mydata.in_graph = &in_graph;
    mydata.logfile  = fopen("cb.log", "w+");
    mydata.timer    = &timer;
  	error = GRBsetcallbackfunc(model, mycallback, (void *) &mydata);
    if (error) error_quit(&env, &model);

  	// Solving the model
#if LOGS == true
    GRBwrite(model, "test.lp");
#endif
  	error = GRBoptimize(model);
  	if (error) error_quit(&env, &model);
    timer.stop();

  	// Obtaining post-optimization information
  	int optimstatus, num_sols;
    double objval, num_nodes, gap;
  	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) error_quit(&env, &model);
    error = GRBgetintattr(model, GRB_INT_ATTR_SOLCOUNT, &num_sols);
    if (error) error_quit(&env, &model);
    error = GRBgetdblattr(model, GRB_DBL_ATTR_NODECOUNT, &num_nodes);
    if (error) error_quit(&env, &model);
    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) error_quit(&env, &model);
    error = GRBgetdblattr(model, GRB_DBL_ATTR_MIPGAP, &gap);
    if (error) error_quit(&env, &model);

#if LOGS == true
  	printf("\nOptimization complete\n");
    if (optimstatus == GRB_OPTIMAL) {
      printf("Optimal objective: %.2lf (%.2lf)\n", objval, timer.getStopTime());
      printf("Linear Relaxation: %.2lf\n", mydata.linear_rel);
      printf("Linear Relaxation (after cuts): %.2lf (%.2lf)\n", mydata.cut_rel, mydata.linear_time);
      printf("# of cuts applied: %d\n", mydata.cuts_applied);
    } else if (optimstatus == GRB_INF_OR_UNBD) {
      printf("Model is infeasible or unbounded\n");
    } else {
      printf("Optimization was stopped early\n");
    }
#endif

    if (optimstatus == GRB_OPTIMAL || optimstatus == GRB_SUBOPTIMAL) {
      ofstream _file;
      _file.open("results.csv", ios::app);
      if(_file.is_open()){
        _file.precision(3);
        _file << fixed << objval << "," << gap << "," << timer.getStopTime() << "," << mydata.linear_rel << "," << mydata.cut_rel << ", ," << mydata.linear_time << "," << num_sols << "," << num_nodes << "," << mydata.cuts_applied << endl;
        _file.close();
      }
    }
    fclose(mydata.logfile);
  } else { // Lagrangian Relaxation
    // Initializing Lagrangian parameters
    double sub_step = 2.0;
    double step_limit = 0.05;
    unsigned it_count = 2; // i.e. two iterations without improvement
    double best_dual = -MAX_DOUBLE;
    double primal_bound = string_to< double >(args[6]); // Primal bound given

    // Lagrangian multipliers
    double* alpha_multipliers = new double[m];
    double* beta_multipliers = new double[n];

    // Initializing Lagrangian multipliers with value 0.0
    memset(alpha_multipliers, 0, sizeof(double) * m);
    memset(beta_multipliers, 0, sizeof(double) * n);

    double current_dual;
    unsigned count = 0, it = 0;
    double* subgradients = new double[numvars];
    double* current_vals = new double[numvars];
    double* best_vals = new double[numvars];
    do {
      // Solving current relaxed model
#if LOGS == true
      GRBwrite(model, "test.lp");
#endif
      GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_OUTPUTFLAG, 0);
      error = GRBoptimize(model);
    	if (error) error_quit(&env, &model);

  		// Retrieving variables & obj function value from the relaxed problem
      GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, numvars, current_vals);
      GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &current_dual);

#if LOGS == true
      printf("----------------------------------\n");
      printf("Subgradient agility: %.2lf\n", sub_step);
      printf("Current dual value: %.2lf\n", current_dual);
      printf("Best dual value: %.2lf\n", best_dual);
#endif

  		if(option == 2){ // Lagrangian Subgradient method for Consts (2) relaxed
#if LOGS == true
        printf("Beta Multiplier: ");
        for(int i = 0; i < n; i++)
          printf("%.2lf ", beta_multipliers[i]);
        printf("\n");
#endif
        // Calculating subgradients
        for(int v = 0; v < n; v++) {
          subgradients[v] = k;
          for(int j = 0; j < R; j++)
            subgradients[v] -= current_vals[v * R + j];
        }
  			// Calculating stepsize
      	double aux = 0.0;
      	for(int v = 0; v < n; v++)
      		aux += subgradients[v] * subgradients[v];
        if(!aux) {
          best_dual = current_dual;
          best_vals = current_vals;
          break;
        }
      	double step_size = ( sub_step * (primal_bound - current_dual) ) / aux;
#if LOGS == true
        printf("Subgradients: ");
        for(int v = 0; v < n; v++)
          printf("%.2lf ", subgradients[v]);
        printf("\nStep size: %.2lf\n", step_size);
#endif

  			// Updating Lagrangian multipliers
  			for(int v = 0; v < n; v++)
  				beta_multipliers[v] = beta_multipliers[v] - ( step_size * subgradients[v] );

  		} else if(option == 3) { // Lagrangian Subgradient method for Consts (3) relaxed
#if LOGS == true
        printf("Alpha Multiplier: ");
        for(int uv = 0; uv < m; uv++)
          printf("%.2lf ", alpha_multipliers[uv]);
        printf("\n");
#endif
        // Calculating subgradients
        for(int uv = 0; uv < m; uv++) {
          subgradients[uv] = c;
          for(int j = 0; j < R; j++)
            subgradients[uv] -= current_vals[(n * R) + uv * R + j];
        }
        // Calculating stepsize
        double aux = 0.0;
        for(int uv = 0; uv < m; uv++)
          aux += subgradients[uv] * subgradients[uv];
        if(!aux) {
          best_dual = current_dual;
          best_vals = current_vals;
          break;
        }
        double step_size = ( sub_step * (primal_bound - current_dual) ) / aux;
#if LOGS == true
        printf("Subgradients: ");
        for(int uv = 0; uv < m; uv++)
          printf("%.2lf ", subgradients[uv]);
        printf("\nStep size: %.2lf\n", step_size);
#endif

        // Updating Lagrangian multipliers
        for(int uv = 0; uv < m; uv++) {
          double aux = alpha_multipliers[uv] - ( step_size * subgradients[uv] );
          alpha_multipliers[uv] = (aux > 0.0) ? aux : 0;
        }
  		}

  		// Saving the best bound
  		if(current_dual > best_dual){
  			best_dual = current_dual;
  			best_vals = current_vals;
  			count = 0;
  		} else count++;

  		// Checking the improvement
  		if(count == it_count){
  			sub_step /= 2.0;
  			count = 0;
  		}

      // Updating the objective function with new multipliers
      if(option == 2) {
        double dual_const = 0.0;
        for(int v = 0; v < n; v++)
          for(int j = 0; j < R; j++)
            val[v * R + j] = beta_multipliers[v];
        for(int v = 0; v < n; v++)
          dual_const -= k * beta_multipliers[v];
        error = GRBsetdblattr(model, GRB_DBL_ATTR_OBJCON, dual_const);
        error = GRBsetdblattrarray(model, GRB_DBL_ATTR_OBJ, 0, n * R, val);
        if (error) error_quit(&env, &model);
      } else if (option == 3) {
        double dual_const = 0.0;
        int counter = 0;
        for(int uv = 0; uv < m; uv++)
          for(int j = 0; j < R; j++)
            val[counter++] = alpha_multipliers[uv];
        for(int uv = 0; uv < m; uv++)
          dual_const -= c * alpha_multipliers[uv];
        error = GRBsetdblattr(model, GRB_DBL_ATTR_OBJCON, dual_const);
        error = GRBsetdblattrarray(model, GRB_DBL_ATTR_OBJ, n * R, m * R, val);
        if (error) error_quit(&env, &model);
      }
      it++;
  	} while( (current_dual < primal_bound) && (sub_step > step_limit) );
    timer.stop();

		printf("\nOptimization complete. Primal bound: %.2lf\n", primal_bound);
    printf("Lagrangian dual: %.2lf (%.2lf)\n", best_dual, timer.getStopTime());
		printf("Number of iterations: %d\n", it);
		printf("Final Subgradient Agility: %.2lf\n", sub_step);

    // Saving results to file
    ofstream _file;
    _file.open("results.csv", ios::app);
    if(_file.is_open()){
      _file.precision(3);
      _file << fixed << primal_bound << "," << best_dual << ", ," << timer.getStopTime() << "," << it << "," << sub_step << endl;
      _file.close();
    }
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
