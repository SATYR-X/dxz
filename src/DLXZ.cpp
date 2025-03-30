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
}