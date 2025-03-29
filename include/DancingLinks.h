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
}

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

