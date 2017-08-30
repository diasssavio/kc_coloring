//============================================================================
// Name        : instance.cpp
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Instance data class implementation
//============================================================================

#include "../include/utils.h"


vector< int > get_adjacency(int v, vector<edge>& edges) {
  vector< int > adjacency;
  for(int i = 0; i < edges.size(); i++)
    if(edges[i].first == v)
      adjacency.push_back(edges[i].second);
    else if (edges[i].second == v)
      adjacency.push_back(edges[i].first);

  return adjacency;
}

bool is_adjacent(int u, int v, vector<edge>& edges) {
  for(int i = 0; i < edges.size(); i++)
    if((edges[i].first == u && edges[i].second == v) || (edges[i].first == v && edges[i].second == u))
      return true;
  return false;
}

vector< int > get_edges_from_clique(vector< int >& clique, vector< edge >& edges) {
  vector< int > to_return;
  for(int e = 0; e < edges.size(); e++)
    if(find(clique.begin(), clique.end(), edges[e].first) != clique.end() && find(clique.begin(), clique.end(), edges[e].second) != clique.end())
      to_return.push_back(e);

  return to_return;
}

bool comp(pair< int, double > i, pair< int, double > j) { return i.second > j.second; }

int __stdcall mycallback(GRBmodel *model, void *cbdata, int where, void *usrdata) {
  c_data *mydata = (c_data*) usrdata;
  int n = mydata->in_graph->get_n();
  int m = mydata->in_graph->get_m();
  int R = mydata->in_graph->get_R();
  int k = mydata->in_graph->get_k();
  int c = mydata->in_graph->get_c();
  vector< edge > edges = mydata->in_graph->get_edges();

  int* ind = new int[mydata->numvars];
	double* val = new double[mydata->numvars];
  int error;
  char name[100];

  if (where == GRB_CB_MIPNODE) {
    /* MIP node callback */
    int status, nodecnt, beta;
    GRBcbget(cbdata, where, GRB_CB_MIPNODE_NODCNT, &nodecnt);

    beta = (!nodecnt) ? 10 : 2;

    GRBcbget(cbdata, where, GRB_CB_MIPNODE_STATUS, &status);
    if (status == GRB_OPTIMAL) {
#if LOGS == true
      fprintf(mydata->logfile, "\n*** Entering callback on node #%d ***\n", nodecnt);
#endif

      // Gathering linear relaxation data
      if(!nodecnt) {
        if(mydata->flag) {
          GRBcbget(cbdata, where, GRB_CB_MIPNODE_OBJBND, &mydata->linear_rel);
          mydata->flag = false;
        } else GRBcbget(cbdata, where, GRB_CB_MIPNODE_OBJBND, &mydata->cut_rel);
      }
      if(!mydata->timer)
        if(nodecnt > 0) {
          mydata->linear_time = ((double) mydata->timer->getMilliSpan() / 1000);
          mydata->timer = NULL;
        }

      // Getting linear relaxation in the current node
      GRBcbget(cbdata, where, GRB_CB_MIPNODE_REL, mydata->solution);

#if LOGS == true
      fprintf(mydata->logfile, "w_j = ");
      for(int j = 0; j < R; j++)
        fprintf(mydata->logfile, "%.2lf ", mydata->solution[(n * R) + (m * R) + j]);
      fprintf(mydata->logfile, "\n");
#endif

      // Starting problem separation
      for(int j = 0; j < R; j++) {
        if(mydata->solution[(n * R) + (m * R) + j] < EPSILON) continue;

#if LOGS == true
        fprintf(mydata->logfile, " ------------------------------------------- \n");
        fprintf(mydata->logfile, "x_v_%d = ", j);
#endif
        vector< pair< int, double > > nz_values;
        for(int v = 0; v < n; v++) {
          if(mydata->solution[v * R + j] > EPSILON)
            nz_values.push_back(make_pair(v, mydata->solution[v * R + j]));
#if LOGS == true
          fprintf(mydata->logfile, "%.2lf ", mydata->solution[v * R + j]);
#endif
        }
#if LOGS == true
        fprintf(mydata->logfile, "\n");
#endif

        // Non-zero decreasing order of variables x*
        sort(nz_values.begin(), nz_values.end(), comp);

#if LOGS == true
        // for(int v = 0; v < nz_values.size(); v++)
        //   fprintf(mydata->logfile, "%4d", nz_values[v].first);
        // fprintf(mydata->logfile, "\n");
#endif

        for(int i = 0; i < nz_values.size(); i++) {
          int v = nz_values[i].first; // Current vertex
          vector< int > adjacency = get_adjacency(v, edges);

          // Doing beta trials of mounting a clique
          int x = 0;
          for(int k1 = 0; k1 < beta; k1++) {
            vector< int > clique;
            clique.push_back(v);

            // Finding a k2 associated with v, i.e. an adjacent vertex
            while(find(adjacency.begin(), adjacency.end(), nz_values[x].first) == adjacency.end() && x < nz_values.size()) ++x;
            if(x == nz_values.size()) break;
            clique.push_back(nz_values[x].first);
            ++x;

            // Trying to expand the clique
            for(int l = x; l < nz_values.size(); l++) {
              bool adjacent = true;
              for(int pqp = 0; pqp < clique.size(); pqp++)
                if(!is_adjacent(clique[pqp], nz_values[l].first, edges)) {
                  adjacent = false;
                  break;
                }
              if(adjacent)
                clique.push_back(nz_values[l].first);
            }

#if LOGS == true
            fprintf(mydata->logfile, "Clique of size %d found, color %d: ", clique.size(), j);
            for(int pqp = 0; pqp < clique.size(); pqp++)
              fprintf(mydata->logfile, "%d (%.2lf) - ", clique[pqp], mydata->solution[clique[pqp] * R + j]);
            fprintf(mydata->logfile, "\n");
#endif
            // Finding the other c colors in the new found maximal clique
            set< int > _colors;
            _colors.insert(j);
            for(int l = 0; l < R; l++) {
              if(l == j) continue;
              for(int pqp = 0; pqp < clique.size(); pqp++)
                if(mydata->solution[clique[pqp] * R + l] > EPSILON && mydata->solution[(n * R) + (m * R) + l] > EPSILON) {
                  _colors.insert(l);
                  break;
                }
              if(_colors.size() == c + 1) break;
            }
            vector< int > colors(_colors.begin(), _colors.end());

#if LOGS == true
            fprintf(mydata->logfile, "Colors found for clique: ");
            for(int pqp = 0; pqp < colors.size(); pqp++)
              fprintf(mydata->logfile, "%4d (%.2lf) - ", colors[pqp], mydata->solution[(n * R) + (m * R) + colors[pqp]]);
            fprintf(mydata->logfile, "\n");
#endif

            // Checking if cuts 2.1 are violated so to be added
            double cut_value = 0.0;
            int counter = 0;
            for(int v = 0; v < clique.size(); v++)
              for(int j0 = 0; j0 < c + 1; j0++) {
                cut_value += mydata->solution[clique[v] * R + colors[j0]];
                ind[counter] = clique[v] * R + colors[j0];
                val[counter] = 1.0;
#if LOGS == true
                fprintf(mydata->logfile, " +%.2lf x_%d_%d", val[counter], clique[v], colors[j0]);
#endif
                ++counter;
              }
            // fprintf(mydata->logfile, "Cut value [1] %.2lf\n", cut_value);
            for(int j0 = 0; j0 < c; j0++) {
              cut_value -= mydata->solution[(n * R) + (m * R) + colors[j0]] * clique.size();
              ind[counter] = (n * R) + (m * R) + colors[j0];
              val[counter] = -1.0 * clique.size();
#if LOGS == true
              fprintf(mydata->logfile, " %.2lf w_%d", val[counter], colors[j0]);
#endif
              ++counter;
            }
            // fprintf(mydata->logfile, "Cut value [2] %.2lf\n", cut_value);
            cut_value -= mydata->solution[(n * R) + (m * R) + colors[c]];
            ind[counter] = (n * R) + (m * R) + colors[c];
            val[counter] = -1.0;
#if LOGS == true
            fprintf(mydata->logfile, " %.2lf w_%d", val[counter], colors[c]);
            // fprintf(mydata->logfile, "Residual cut value of %.2lf\n", cut_value);
#endif
            if(cut_value > EPSILON) { // Cut violated
              error = GRBcbcut(cbdata, ++counter, ind, val, '<', 0);
#if LOGS == true
              // fprintf(mydata->logfile, "Violated cut\n");
              fprintf(mydata->logfile, " <= 0\n");
#endif
              if(!error)
                mydata->cuts_applied++;
            }
#if LOGS == true
            else fprintf(mydata->logfile, "\n");
#endif
            // Checking if cuts 2.2 are violated so to be added
            cut_value = 0.0;
            counter = 0;
            for(int v = 0; v < clique.size(); v++) {
              cut_value += mydata->solution[clique[v] * R + colors[0]] * clique.size();
              ind[counter] = clique[v] * R + colors[0];
              val[counter] = clique.size();
              ++counter;
#if LOGS == true
              fprintf(mydata->logfile, " +%.2lf x_%d_%d", val[counter], clique[v], colors[0]);
#endif
            }
            vector< int > clique_edges = get_edges_from_clique(clique, edges);
            for(int uv = 0; uv < clique_edges.size(); uv++) {
              cut_value -= mydata->solution[(n * R) + clique_edges[uv] * R + colors[0]];
              ind[counter] = (n * R) + clique_edges[uv] * R + colors[0];
              val[counter] = -1.0;
              ++counter;
#if LOGS == true
              fprintf(mydata->logfile, " %.2lf y_%d_%d", val[counter], clique_edges[uv], colors[0]);
#endif
            }
            for(int j0 = 1; j0 < colors.size(); j0++)
              for(int uv = 0; uv < clique_edges.size(); uv++) {
                cut_value += mydata->solution[(n * R) + clique_edges[uv] * R + colors[j0]];
                ind[counter] = (n * R) + clique_edges[uv] * R + colors[j0];
                val[counter] = 1.0;
                ++counter;
#if LOGS == true
                fprintf(mydata->logfile, " +%.2lf y_%d_%d", val[counter], clique_edges[uv], colors[j0]);
#endif
              }
            double constant = ((c * clique.size() * (clique.size() - 1)) / 2.0) + clique.size();
            cut_value -= constant * mydata->solution[(n * R) + (m * R) + colors[0]];
            ind[counter] = (n * R) + (m * R) + colors[0];
            val[counter] = -1.0 * constant;
#if LOGS == true
            fprintf(mydata->logfile, " %.2lf w_%d", val[counter], colors[0]);
#endif
            if(cut_value > EPSILON) {
              error = GRBcbcut(cbdata, ++counter, ind, val, '<', 0);
#if LOGS == true
              fprintf(mydata->logfile, " <= 0\n");
#endif
              if(!error)
                mydata->cuts_applied++;
            }
#if LOGS == true
            else fprintf(mydata->logfile, "\n");
#endif
          }
        }
      }
    }
  }

  free(ind);
  free(val);

  return 0;
}
