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

// Testing generation functions and FibonacciHeap class
int main() {
    int n = 100;

    for (const std::string& density : {"sparse", "dense"}) {
        std::vector<std::vector<int>> matrix;

        auto graph = generateGraph(
            n, density, 1, 100, matrix
        );

        auto fw = floydWarshall(matrix, n);
        auto bf = bellmanFord(graph, n, 0);

        std::cout << density << " Floyd-Warshall: "
                  << (fw.empty() ? "FAILED" : "OK") << "\n";

        std::cout << density << " Bellman-Ford: "
                  << (bf.empty() ? "FAILED" : "OK") << "\n";

        FibonacciHeap heap;

        heap.insert(0, 10);
        heap.insert(1, 5);
        heap.insert(2, 20);

        FibNode* min = heap.extractMin();

        std::cout << density << " Fibonacci Heap: "
                  << (min && min->distance == 5 ? "OK" : "FAILED")
                  << "\n";

        delete min;
    }

    return 0;
}