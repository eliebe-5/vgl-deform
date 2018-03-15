#include "volmesh.h"
#include <vector>

void weigh_nodes(Mesh*);
void deform(Mesh* mesh, std::vector<int>&, std::vector<v3>);
void select_edge(Mesh*, v3, v3, std::vector<int>&);
void clear_edges(std::vector<int>& selected);
