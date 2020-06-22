#pragma once

#include <algorithm>
#include <fstream>
#include <vector>

class Dijkstra
{
public:
    std::vector<std::vector<int>> graph;
    std::vector<int> distances;
    int size;
    int start;

    Dijkstra();

    void writeResult();
    int min_distance(const std::vector<int>& dist, const std::vector<bool>& used_set);
    void sequentalDijkstra();

    void parallelStaticDijkstra();
    void parallelDynamicDijkstra();

};
