//============================================================================
// Name        : instance_generator.cpp
// Author      : Sávio S. Dias
// E-mail      : diasssavio@gmail.com
// Institution : Universidade Federal do Rio de Janeiro
// Description : Instance Generator Program for (k,c)-coloring problem based in Zabala(2015)
//============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <vector>
#include <algorithm>
#include "include/typedef.hpp"
#include "include/mt19937ar.h"

using namespace std;

bool mycomp(edge e1, edge e2) {
    if(e1.first == e2.first) return e1.second < e2.second;
    return e1.first < e2.first;
}

int gen_20();
int gen_larger(unsigned);

int main(int argc, char *args[]) {
    unsigned long seed = time(NULL);
    if (argc >= 3)
        seed = string_to< unsigned long >(args[2]);
    init_genrand(seed);

    if (string_to< int >(args[1]) == 1){
        FILE *fptr = fopen("instances/generated/set1/seed.txt", "w+"); // Writing files
        if (fptr) {
            fprintf(fptr, "%ld\n", seed);
            fclose(fptr);
            return gen_20();
        }
    } else if (string_to<int>(args[1]) >= 2) {
        FILE *fptr = fopen("instances/generated/seed.txt", "w+"); // Writing files
        if (!fptr)
        {
            fprintf(fptr, "%ld\n", seed);
            fclose(fptr);
            gen_larger(string_to<int>(args[1]));
        }
    }

    return 0;
}

int gen_20() {
    unsigned n = 20;
    unsigned m = n * n;

    for(unsigned i = 1; i <= 9; i++) { // edge probabilities
        for(unsigned inst = 0; inst < 10; inst++) { // number of instances per prob
            vector< edge > choosen;
            vector< int > edges_index(m);
            for(int e1 = 0; e1 < m; e1++) edges_index[e1] = e1;
            for (int v1 = 0; v1 < n; v1++)
                edges_index.erase(find(edges_index.begin(), edges_index.end(), v1 * n + v1));
            for(unsigned j = 0; j < (i * 0.1 * (n * (n-1) / 2)); j++) { // choosing edges at random
                int e = genrand_int32() % edges_index.size();
                int v1, v2;
                for(v1 = 0; v1 < n; v1++) {
                    bool found = false;
                    for(v2 = 0; v2 < n; v2++)
                        if((v1 * n + v2) == edges_index[e]) {
                            found = true;
                            break;
                        }
                    if(found) break;
                }
                edges_index.erase(edges_index.begin() + e);
                edges_index.erase(find(edges_index.begin(), edges_index.end(), v2 * n + v1));
                choosen.push_back(make_pair(v1, v2));
            }
            sort(choosen.begin(), choosen.end(), mycomp);
            for(unsigned k = 2; k <= 5; k++) // choosing (k,c) pairs
                for(unsigned c = 1; c < k; c++) {
                    char filename[256];
                    if(i <= 3)
                        sprintf(filename, "instances/generated/set1/low/kc%d_%d%d_%d.txt", i * 10, k, c, inst + 1);
                    else if(i <= 6)
                        sprintf(filename, "instances/generated/set1/medium/kc%d_%d%d_%d.txt", i * 10, k, c, inst + 1);
                    else
                        sprintf(filename, "instances/generated/set1/high/kc%d_%d%d_%d.txt", i * 10, k, c, inst + 1);

                    printf("Generating %s... ", filename);
                    FILE *fptr = fopen(filename, "w+"); // Writing files
                    if(!fptr) {
                        printf("ERROR: Unable to open file %s", filename);
                        return 0;
                    }
                    fprintf(fptr, "%d %d %d %d\n", n, choosen.size(), k, c);
                    for(unsigned e = 0; e < choosen.size(); e++)
                        fprintf(fptr, "%d %d\n", choosen[e].first, choosen[e].second);
                    fclose(fptr);
                    printf("OK!\n");
                }
        }
    }

    return 0;
}

int gen_larger(unsigned n) {
    return 1;
}