#include "DancingLinks.h"

DancingLinks::DancingLinks( 
    int rows, 
    int cols, 
    const std::vector<std::vector<int>>& matrix
    ) : ROWS(rows), COLS(cols), countNum(0), countSolution(0){
        T = new ZDDNode(-1,nullptr,nullptr,true);
        F = new ZDDNode(-2,nullptr,nullptr,true);

        ColIndex = new ColumnHeader[cols+1];
        RowIndex = new Node[rows];
        root = &ColIndex[0];
        ColIndex[0].left = &ColIndex[COLS];
        ColIndex[0].right = &ColIndex[1];
        ColIndex[COLS].right = &ColIndex[0];
        ColIndex[COLS].left = &ColIndex[COLS-1];
        for( int i=1; i<cols; i++ )
        {
            ColIndex[i].left = &ColIndex[i-1];
            ColIndex[i].right = & ColIndex[i+1];
        }

        for( int i=0; i<=cols; i++ )
        {
            ColIndex[i].up = &ColIndex[i];
            ColIndex[i].down = &ColIndex[i];
            ColIndex[i].col = i;
        }
        ColIndex[0].down = &RowIndex[0];

        for( int i=0; i<rows; i++ ){
            for( int j=0; j<cols; j++){
                if(matrix[i][j] == 1){
                    insert( i, j + 1 );
                }
            }
        }
    }

DancingLinks::~DancingLinks()
{
    for( int i=1; i<=COLS; i++ )
    {
        Node* cur = ColIndex[i].down;
        Node* del = cur->down;
        while( cur != &ColIndex[i] ){
            delete cur;
            cur = del;
            del = cur->down;
        }
    }
    delete[] RowIndex;
    delete[] ColIndex;

    std::unordered_set<ZDDNode*> deletedNodes;

    for (auto& pair : Z) {
        ZDDNode* node = pair.second;
        if (node != nullptr && deletedNodes.find(node) == deletedNodes.end()){
            delete node;
            deletedNodes.insert(node);
        }
    }
    Z.clear();

    for(auto& pair : C) {
        ZDDNode* node = pair.second;
        if (node != nullptr && deletedNodes.find(node) == deletedNodes.end()){
            delete node;
            deletedNodes.insert(node);
        }
    }
    C.clear();
}

void DancingLinks::insert(int r, int c)
{
    Node* cur = &ColIndex[c];
    ColIndex[c].size++;
    Node* newNode = new Node( r, c );
    while( cur->down != &ColIndex[c] && cur->down->row < r )
        cur = cur->down;
    newNode->down = cur->down;
    newNode->up = cur;
    cur->down->up = newNode;
    cur->down = newNode;
    if( RowIndex[r].right == NULL ){
        RowIndex[r].right = newNode;
        newNode->left = newNode;
        newNode->right = newNode;
    }
    else{
        Node* rowHead = RowIndex[r].right;
        cur = rowHead;
        while( cur->right != rowHead && cur->right->col < c)
            cur = cur->right;
        newNode->right = cur->right;
        newNode->left = cur;
        cur->right->left = newNode;
        cur->right = newNode;
    }
}

void DancingLinks::cover( int c )
{
    ColumnHeader* col = &ColIndex[c];
    col->right->left = col->left;
    col->left->right = col->right;
    Node* curR, *curC;
    curC = col->down;
    while ( curC != col )
    {
        Node* noteR = curC;
        curR = noteR->right;
        while( curR != noteR ){
            curR->down->up = curR->up;
            curR->up->down = curR->down;
            ColIndex[curR->col].size--;
            curR = curR->right;
        }
        curC = curC->down;
    }
    
}

void DancingLinks::uncover( int c )
{
    Node* curR, *curC;
    ColumnHeader* col = &ColIndex[c];
    curC = col->up;
    while( curC != col )
    {
        Node* noteR = curC;
        curR = curC->left;
        while( curR != noteR )
        {
            ColIndex[curR->col].size++;
            curR->down->up = curR;
            curR->up->down =curR;
            curR = curR->left;
        }
        curC = curC->up;
    }
    col->right->left = col;
    col->left->right = col;
}

void DancingLinks::columnToVector(std::vector<bool>& vec)
{
    ColumnHeader* cur = root;
    while( cur->right != root ){
        vec[cur->right->col - 1] = true;
        cur = (ColumnHeader*)cur->right;
    }
}

bool areSubtreesEqual(ZDDNode* s, ZDDNode* t){
    if(!s && !t)return true;

    if(!s || !t)return false;

    if(s->label != t->label)return false;

    if(!areSubtreesEqual(s->lo, t->lo)) return false;

    if(!areSubtreesEqual(s->hi, t->lo)) return false;

    return true;
}

ZDDNode* findSubtree(ZDDNode* s, ZDDNode* t) {
   if(!s) return nullptr;

   if(areSubtreesEqual(s, t)){
    return s;
   } 

   // 在左子树中查找
    ZDDNode* leftResult = findSubtree(s->lo, t);
    if (leftResult) {
        return leftResult; // 在左子树中找到了子树
    }
    // 在右子树中查找
    ZDDNode* rightResult = findSubtree(s->hi, t);
    if (rightResult) {
        return rightResult; // 在右子树中找到了子树
    }
    return nullptr; // 没有找到子树
}
