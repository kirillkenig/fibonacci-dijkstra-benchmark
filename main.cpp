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

// Fibonacci Heap
struct FibNode {
    int vertex;
    int distance;
    int degree;
    bool marked;
    FibNode* parent;
    FibNode* child;
    FibNode* left;
    FibNode* right;

    FibNode(int v, int dist)
        : vertex(v), distance(dist), degree(0), marked(false),
          parent(nullptr), child(nullptr), left(this), right(this) {}
};

class FibonacciHeap {
private:
    FibNode* minNode;
    int nodeCount;

    void link(FibNode* y, FibNode* x) {
        y->left->right = y->right;
        y->right->left = y->left;

        y->parent = x;

        if (!x->child) {
            x->child = y;
            y->left = y->right = y;
        } else {
            y->left = x->child;
            y->right = x->child->right;
            x->child->right->left = y;
            x->child->right = y;
        }

        x->degree++;
        y->marked = false;
    }

    void consolidate() {
        if (!minNode || nodeCount <= 1)
            return;

        int maxDegree =
            static_cast<int>(std::log2(std::max(2, nodeCount))) + 2;

        std::vector<FibNode*> degreeTable(maxDegree, nullptr);
        std::vector<FibNode*> rootList;

        FibNode* current = minNode;

        do {
            rootList.push_back(current);
            current = current->right;
        } while (current != minNode);

        for (FibNode* node : rootList) {
            int d = node->degree;
            FibNode* x = node;

            while (degreeTable[d] != nullptr) {
                FibNode* y = degreeTable[d];

                if (x->distance > y->distance)
                    std::swap(x, y);

                link(y, x);

                degreeTable[d] = nullptr;
                d++;
            }

            degreeTable[d] = x;
        }

        minNode = nullptr;

        for (FibNode* node : degreeTable) {
            if (!node)
                continue;

            node->left = node->right = node;

            if (!minNode) {
                minNode = node;
            } else {
                node->left = minNode;
                node->right = minNode->right;
                minNode->right->left = node;
                minNode->right = node;

                if (node->distance < minNode->distance)
                    minNode = node;
            }
        }
    }

      void decreaseKeyInternal(FibNode* node, int newDist) {
        node->distance = newDist;

        FibNode* parent = node->parent;

        if (parent && node->distance < parent->distance) {
            cut(node, parent);
            cascadingCut(parent);
        }

        if (node->distance < minNode->distance)
            minNode = node;
    }

    void cut(FibNode* node, FibNode* parent) {
        parent->degree--;

        if (node->right == node) {
            parent->child = nullptr;
        } else {
            node->left->right = node->right;
            node->right->left = node->left;

            if (parent->child == node)
                parent->child = node->right;
        }

        node->parent = nullptr;
        node->marked = false;

        node->left = minNode;
        node->right = minNode->right;
        minNode->right->left = node;
        minNode->right = node;
    }

    void cascadingCut(FibNode* node) {
        FibNode* parent = node->parent;

        if (parent) {
            if (!node->marked) {
                node->marked = true;
            } else {
                cut(node, parent);
                cascadingCut(parent);
            }
        }
    }

public:
    FibonacciHeap()
        : minNode(nullptr), nodeCount(0) {}

    FibNode* insert(int vertex, int distance) {
        FibNode* node = new FibNode(vertex, distance);

        if (!minNode) {
            minNode = node;
        } else {
            node->left = minNode;
            node->right = minNode->right;
            minNode->right->left = node;
            minNode->right = node;

            if (node->distance < minNode->distance)
                minNode = node;
        }

        nodeCount++;

        return node;
    }

    FibNode* extractMin() {
        FibNode* z = minNode;

        if (!z)
            return nullptr;

        if (z->child) {
            FibNode* c = z->child;

            do {
                FibNode* next = c->right;

                c->parent = nullptr;
                c->marked = false;

                c->left = minNode;
                c->right = minNode->right;
                minNode->right->left = c;
                minNode->right = c;

                c = next;
            } while (c != z->child);
        }

        if (z == z->right) {
            minNode = nullptr;
        } else {
            z->left->right = z->right;
            z->right->left = z->left;

            minNode = z->right;
            consolidate();
        }

        nodeCount--;

        z->left = z->right = z;

        return z;
    }

    void decreaseKey(FibNode* node, int newDist) {
        if (!node || newDist > node->distance)
            return;

        decreaseKeyInternal(node, newDist);
    }

    bool empty() const {
        return nodeCount == 0;
    }
};

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

    long long targetEdges =
        (densityType == "sparse") ? 2LL*n : 1LL*n*(n-1);

    long long edgesAdded = 0;

    while (edgesAdded < targetEdges) {
        int u = vertexDist(gen);
        int v = vertexDist(gen);

        if (u == v || denseMatrix[u][v] != INF)
            continue;

        int w = weightDist(gen);

        adjList[u].push_back({v, w});
        denseMatrix[u][v] = w;

        edgesAdded++;
    }

    return adjList;
}

// Floyd-Warshall
std::vector<std::vector<int>> floydWarshall(
    const std::vector<std::vector<int>>& distInit, int n
) {
    std::vector<std::vector<int>> dist = distInit;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            if (dist[i][k] != INF)
                for (int j = 0; j < n; ++j)
                    if (dist[k][j] != INF) {
                        long long candidate =
                            static_cast<long long>(dist[i][k]) + dist[k][j];

                        if (candidate < dist[i][j])
                            dist[i][j] = static_cast<int>(candidate);
                    }

    for (int i = 0; i < n; ++i)
        if (dist[i][i] < 0) {
            std::cout << "Negative cycle detected in Floyd-Warshall\n";
            return {};
        }

    return dist;
}

// Bellman-Ford
std::vector<int> bellmanFord(
    const std::vector<std::vector<std::pair<int,int>>>& graph,
    int n,
    int source
) {
    std::vector<int> dist(n, INF);
    dist[source] = 0;

    for (int i = 0; i < n-1; ++i)
        for (int u = 0; u < n; ++u)
            for (auto &e : graph[u]) {
                int v = e.first;
                int w = e.second;

                if (dist[u] != INF && dist[u] + w < dist[v])
                    dist[v] = dist[u] + w;
            }

    for (int u = 0; u < n; ++u)
        for (auto &e : graph[u]) {
            int v = e.first;
            int w = e.second;

            if (dist[u] != INF && dist[u] + w < dist[v])
                return {};
        }

    return dist;
}

// Dijkstra via Fib Heap
std::vector<int> dijkstraStandard(
    const std::vector<std::vector<std::pair<int,int>>>& graph,
    int n,
    int startNode
) {
    std::vector<int> distances(n, INF);
    std::vector<FibNode*> nodes(n, nullptr);
    FibonacciHeap pq;

    distances[startNode] = 0;
    nodes[startNode] = pq.insert(startNode, 0);

    while (!pq.empty()) {
        FibNode* minNode = pq.extractMin();

        if (!minNode)
            break;

        int u = minNode->vertex;
        int currDist = minNode->distance;

        delete minNode;
        nodes[u] = nullptr;

        if (currDist > distances[u])
            continue;

        for (auto &e : graph[u]) {
            int v = e.first;
            int w = e.second;

            if (distances[u] != INF &&
                distances[u] + w < distances[v]) {

                distances[v] = distances[u] + w;

                if (!nodes[v]) {
                    nodes[v] = pq.insert(v, distances[v]);
                } else {
                    pq.decreaseKey(nodes[v], distances[v]);
                }
            }
        }
    }

    return distances;
}

// Johnson
std::vector<std::vector<int>> johnsonsAlgorithm(
    const std::vector<std::vector<std::pair<int,int>>>& graph,
    int n
) {
    std::vector<std::vector<std::pair<int,int>>> gPrime = graph;

    gPrime.push_back(std::vector<std::pair<int,int>>());

    for (int i = 0; i < n; ++i)
        gPrime[n].push_back({i, 0});

    std::vector<int> h = bellmanFord(gPrime, n + 1, n);

    if (h.empty())
        return {};

    std::vector<std::vector<std::pair<int,int>>> reweighted(n);

    for (int u = 0; u < n; ++u)
        for (auto &e : graph[u]) {
            int v = e.first;
            int w = e.second;

            if (h[u] != INF && h[v] != INF)
                reweighted[u].push_back({
                    v,
                    w + h[u] - h[v]
                });
        }

    std::vector<std::vector<int>> allDist(
        n,
        std::vector<int>(n, INF)
    );

    for (int u = 0; u < n; ++u) {
        std::vector<int> dist =
            dijkstraStandard(reweighted, n, u);

        for (int v = 0; v < n; ++v)
            if (dist[v] != INF &&
                h[u] != INF &&
                h[v] != INF) {

                allDist[u][v] =
                    dist[v] + h[v] - h[u];
            }
    }

    return allDist;
}

void runSimulation(
    const std::vector<int>& vertexCounts,
    const std::vector<std::string>& density,
    int numRuns,
    int minW,
    int maxW
) {
    std::cout << std::left
              << std::setw(15) << "Vertices (n)"
              << " | " << std::setw(15) << "Edges (m)"
              << " | " << std::setw(15) << "Density"
              << " | " << std::setw(25) << "Floyd-Warshall (ms)"
              << " | " << std::setw(25) << "Johnson (ms)"
              << std::endl;

    std::cout << std::string(110, '-') << std::endl;

    for (int n : vertexCounts) {
        for (const std::string& densityType : density) {
            double fwTotal = 0;
            double johnsonTotal = 0;

            long long actualEdges = 0;
            int johnsonSuccess = 0;

            for (int run = 0; run < numRuns; ++run) {
                std::vector<std::vector<int>> denseMatrix;

                auto adjList =
                    generateGraph(
                        n,
                        densityType,
                        minW,
                        maxW,
                        denseMatrix
                    );

                if (run == 0) {
                    for (auto &edges : adjList)
                        actualEdges += edges.size();
                }

                auto startFw =
                    std::chrono::high_resolution_clock::now();

                auto fwResult =
                    floydWarshall(denseMatrix, n);

                auto endFw =
                    std::chrono::high_resolution_clock::now();

                fwTotal +=
                    std::chrono::duration<
                        double,
                        std::milli
                    >(endFw - startFw).count();

                auto startJ =
                    std::chrono::high_resolution_clock::now();

                auto johnsonResult =
                    johnsonsAlgorithm(adjList, n);

                auto endJ =
                    std::chrono::high_resolution_clock::now();

                if (!johnsonResult.empty()) {
                    johnsonTotal +=
                        std::chrono::duration<
                            double,
                            std::milli
                        >(endJ - startJ).count();

                    johnsonSuccess++;
                }
            }

            double avgFw = fwTotal / numRuns;

            double avgJ =
                (johnsonSuccess > 0)
                    ? johnsonTotal / johnsonSuccess
                    : std::numeric_limits<double>::infinity();

            std::cout << std::left
                      << std::setw(15) << n
                      << " | " << std::setw(15) << actualEdges
                      << " | " << std::setw(15) << densityType
                      << " | " << std::setw(25)
                      << std::fixed
                      << std::setprecision(4)
                      << avgFw
                      << " | " << std::setw(25);

            if (avgJ ==
                std::numeric_limits<double>::infinity()) {

                std::cout << "Negative Cycle / INF";
            } else {
                std::cout << std::fixed
                          << std::setprecision(4)
                          << avgJ;
            }

            std::cout << std::endl;
        }
    }
}

// Testing
int main() {
    std::vector<int> vertexCounts = {
        100, 200, 300, 400, 500
    };

    std::vector<std::string> densities = {
        "sparse", "dense"
    };

    std::cout << "Starting simulation...\n";

    runSimulation(
        vertexCounts,
        densities,
        50,
        1,
        10
    );

    std::cout << "Simulation finished.\n";

    return 0;
}