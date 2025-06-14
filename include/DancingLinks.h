#ifndef DANCINGLINKS_H
#define DANCINGLINKS_H

#include <bitset>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <iostream>
#include <chrono>
#include <atomic>
#include <vector>
#include <string>
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
    int ROWS, COLS;
    int countNum;
    int countSolution;
    bool timeoutEnabled;
    double timeoutSeconds;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    DancingLinks(int rows, int cols, const std::vector<std::vector<int>>& matrix);
    ~DancingLinks();
    void insert(int r, int c);
    void cover(int c);
    void uncover(int c);
    void columnToVector(std::vector<bool>& vec);
    size_t hashFunction(int r, ZDDNode* x, ZDDNode* y);
    ZDDNode* unique(int r, ZDDNode* x, ZDDNode* y);
    ZDDNode* search();
    ZDDNode* searchWithoutCache();
    ZDDNode* searchWithCache();
    void printZDD(ZDDNode* node);
    void printTable();
    void printCache();
    void printColumnHeaders();
    void printRowNodes();
    void printRemainingColumns();
    void printZDDStructure(ZDDNode* node, int depth = 0, std::string prefix = "");
    void printZDDDotFormat(ZDDNode* node, std::ostream& out = std::cout);
    string getColumnState() const;
    
    // 添加公共方法来获取Z和C的大小
    size_t getZTableSize() const { return Z.size(); }
    size_t getCacheSize() const { return C.size(); }
    
    // 超时相关方法
    void setTimeout(double seconds);
    bool isTimeout() const;
    void resetTimeout();
    double getElapsedTime() const;
    int countSolutionsInZDD(ZDDNode* node);  // 计算ZDD中解的数量
private:
    Node* RowIndex;          // 行索引数组，存储每行的第一个节点
    ColumnHeader* root;      // 列头链表的根节点
    ColumnHeader* ColIndex; // 列头数组，存储每列的头节点
    ZDDNode* T;             // ZDD的True终端节点
    ZDDNode* F;             // ZDD的False终端节点
    std::unordered_map<size_t, ZDDNode*> Z;  // ZDD节点唯一化表
                                             // key: 节点的哈希值
                                             // value: 对应的ZDD节点
                                             // 用于确保相同结构的ZDD节点只创建一次
    std::unordered_map<std::string,ZDDNode*> C;  // 列状态缓存表
                                                 // key: 列状态的字符串表示
                                                 // value: 对应列状态下的ZDD节点
                                                 // 用于缓存已计算过的列状态对应的ZDD节点，避免重复计算
    
    // 超时相关成员变量
    std::unordered_map<ZDDNode*, int> solutionCount;
};

#endif