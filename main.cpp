#include <iostream>
#include <vector>
#include <limits>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
#include <cstdint>
#include <cmath>

const int INF = std::numeric_limits<int>::max();

// Graph Generation
std::vector<std::vector<std::pair<int,int>>> generateGraph(
    int n, const std::string& densityType, int minW, int maxW,
    std::vector<std::vector<int>>& denseMatrix
) {
    std::vector<std::vector<std::pair<int,int>>> adjList(n);
    denseMatrix.assign(n, std::vector<int>(n, INF));

    for (int i = 0; i < n; ++i)
        denseMatrix[i][i] = 0;

    std::random_device rd;
    std::mt19937 gen(rd());

    if (minW > maxW)
        std::swap(minW, maxW);
    std::uniform_int_distribution<> weightDist(minW, maxW);

    std::uniform_int_distribution<> vertexDist(0, n-1);

    long long targetEdges = (densityType == "sparse") ? 2LL*n : 1LL*n*(n-1);
    long long edgesAdded = 0;

    while (edgesAdded < targetEdges) {
        int u = vertexDist(gen);
        int v = vertexDist(gen);
        if (u==v || denseMatrix[u][v]!=INF) continue;
        int w = weightDist(gen);
        adjList[u].push_back({v,w});
        denseMatrix[u][v] = w;
        edgesAdded++;
    }

    return adjList;
}

// Floyd-Warshall
std::vector<std::vector<int>> floydWarshall(const std::vector<std::vector<int>>& distInit, int n) {
    std::vector<std::vector<int>> dist = distInit;

    for (int k=0; k<n; ++k)
        for (int i=0; i<n; ++i)
            if (dist[i][k]!=INF)
                for (int j=0; j<n; ++j)
                    if (dist[k][j]!=INF) {
                        long long candidate =
                            static_cast<long long>(dist[i][k]) + dist[k][j];

                        if (candidate < dist[i][j])
                            dist[i][j] = static_cast<int>(candidate);
                    }

    for (int i=0; i<n; ++i)
        if (dist[i][i]<0) {
            std::cout << "Negative cycle detected in Floyd-Warshall\n";
            return {};
        }

    return dist;
}

// Bellman-Ford
std::vector<int> bellmanFord(const std::vector<std::vector<std::pair<int,int>>>& graph, int n, int source) {
    std::vector<int> dist(n, INF);
    dist[source] = 0;

    for (int i=0; i<n-1; ++i)
        for (int u=0; u<n; ++u)
            for (auto &e : graph[u]) {
                int v = e.first, w = e.second;
                if (dist[u]!=INF && dist[u]+w<dist[v])
                    dist[v] = dist[u]+w;
            }

    for (int u=0; u<n; ++u)
        for (auto &e : graph[u]) {
            int v = e.first, w = e.second;
            if (dist[u]!=INF && dist[u]+w<dist[v]) return {};
        }

    return dist;
}
// Testing generation functions
int main() {
    int n = 100;

    for (const std::string& density : {"sparse", "dense"}) {
        std::vector<std::vector<int>> matrix;
        auto graph = generateGraph(n, density, 1, 100, matrix);

        auto fw = floydWarshall(matrix, n);
        auto bf = bellmanFord(graph, n, 0);

        std::cout << density << " Floyd-Warshall: "
                  << (fw.empty() ? "FAILED" : "OK") << "\n";

        std::cout << density << " Bellman-Ford: "
                  << (bf.empty() ? "FAILED" : "OK") << "\n";
    }

    return 0;
}