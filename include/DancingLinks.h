#ifndef DANCINGLINKS_H
#define DANCINGLINKS_H

#include <bitset>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <iostream>
using namespace std;

struct Node {
    Node* left;
    Node* right;
    Node* up;
    Node* down;
    int row;
    int col;
    Node(){
        left = right = up = down = NULL;
        col = row = 0;
    };
    Node(int r, int c){
        left = right = up = down = NULL;
        col = c;
        row = r;
    };
};

struct ColumnHeader : public Node {
    int size;
    ColumnHeader(){
        size = 0;
    }
};

struct ZDDNode {
    int label;
    ZDDNode* lo;
    ZDDNode* hi;
    bool isTerminal;

    ZDDNode(int label, ZDDNode* lo, ZDDNode* hi, bool isTerminal = false)
        : label(label), lo(lo), hi(hi), isTerminal(isTerminal) {}

    ZDDNode(const ZDDNode& other) 
        : label(other.label), isTerminal(other.isTerminal), lo(other.lo), hi(other.hi) {}
};

inline ZDDNode* copyNode(ZDDNode* original) {
    if (original == nullptr){
        return nullptr;
    }

    ZDDNode* newNode = new ZDDNode(*original);

    if (newNode->lo != nullptr) {
        newNode->lo = copyNode(newNode->lo);
    }
    if (newNode->hi != nullptr) {
        newNode->hi = copyNode(newNode->hi);
    }
    return newNode;
}

struct Greater {
    bool operator()(int a, int b) const{
        return a > b;
    }
};

class DancingLinks
{
public:
    int countNum;
    int countSolution;
    DancingLinks(int rows, int cols, const std::vector<std::vector<int>>& matrix);
    ~DancingLinks();
    void insert(int r, int c);
    void cover(int c);
    void uncover(int c);
    void columnToVector(std::vector<bool>& vec);
    size_t hashFunction(int r, ZDDNode* x, ZDDNode* y);
    ZDDNode* unique(int r, ZDDNode* x, ZDDNode* y);
    ZDDNode* search();
    void printZDD(ZDDNode* node);
    void printTable();
    void printCache();
    void printColumnHeaders();
    void printRowNodes();
    void printRemainingColumns();
    string getColumnState() const;

private:
    Node* RowIndex;
    ColumnHeader* root;
    int ROWS;
    int COLS;
    ColumnHeader* ColIndex;
    ZDDNode* T;
    ZDDNode* F;
    std::unordered_map<size_t, ZDDNode*> Z;
    std::unordered_map<std::string,ZDDNode*> C;
};

#endif