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

void DancingLinks::printZDD(ZDDNode* node){
    if(!node) return;

    std::queue<ZDDNode*> q;
    q.push(node);

    std::set<int> visited;

    while(!q.empty()) {
        ZDDNode* current = q.front();
        q.pop();

        if(!current->isTerminal && visited.find(current->label) == visited.end()) {
            std::cout<<"Label: "<<current->label<<"{";
            std::cout<<"Lo: "<< (current->lo ? std::to_string(current->lo->label) : "null")
            << "Hi: "<< (current->hi ? std::to_string(current->hi->label) : "null") 
            << " }" << std::endl;
            visited.insert(current->label);
        }

        if (current->lo) q.push(current->lo);
        if (current->hi) q.push(current->hi);
    }
}

size_t DancingLinks::hashFunction(int r, ZDDNode* x, ZDDNode* y)
{
    return std::hash<int>()(r) ^ (std::hash<int>()(x->label) << 1) ^ (std::hash<int>()(y->label) << 2);
}

ZDDNode* DancingLinks::unique(int r, ZDDNode* x, ZDDNode* y){
    countNum++;

    std::size_t key = hashFunction(r, x, y);
    if (Z.find(key) == Z.end()) {
        ZDDNode* lo = x;
        ZDDNode* hi = y;

        if(x->isTerminal && y->isTerminal){
            Z[key] = new ZDDNode(r, lo, hi);
            return Z[key];
        }

        if(!x->isTerminal && !y->isTerminal){
            if (C.find(getColumnState()) != C.end()) {
                hi = C[getColumnState()];
            }

            Z[key] = new ZDDNode(r, lo, hi);
            return Z[key];
        }

        if(x->isTerminal) {
            lo = F;
        } else if (y->isTerminal) {
            hi = T;
        }

        Z[key] = new ZDDNode(r, lo, hi);
    }
    return Z[key];
}

std::string DancingLinks::getColumnState() const{
    std::string columnState(COLS, '0');
    ColumnHeader* cur = (ColumnHeader*)root->right;
    while (cur != root) {
        columnState[cur->col - 1] = '1';
        cur = (ColumnHeader*)cur->right;
    }
    return columnState;
}

ZDDNode* DancingLinks::search()
{
    if(root->right == root){
        countSolution++;
        return T;
    }

    std::string columnState = getColumnState();

    if (C.find(columnState) != C.end()) {
        countSolution++;
        return C[columnState];
    }

    ColumnHeader* choose = (ColumnHeader*)root->right;
    ColumnHeader* cur = choose;
    while( cur != root )
    {
        if( choose->size > cur->size ) choose = cur;
        cur = (ColumnHeader*)cur->right;
    }

    if( choose->size <= 0 ){
        return F;
    }
    ZDDNode* x = F;

    cover(choose->col);
    Node* curC = choose->up;
    while( curC != choose )
    {
        //printColumnHeaders();
        Node* noteR = curC;
        Node* curR = curC->right;
        while( curR != noteR )
        {
            cover( curR->col );
            curR = curR->right;
        }
        ZDDNode* y = search();
        if( y->label != -2 ){
            x = unique(curC->row, x, y);
        }
        //printColumnHeaders();
        curR = noteR->left;
        while( curR != noteR )
        {
            uncover( curR->col );
            curR = curR->left;
        }
        curC = curC->up;
    }
    uncover(choose->col);
    C[columnState] = x;
    return x;
}

void DancingLinks::printTable(){
    std::cout<<"Table:"<<std::endl;
    for (const auto& pair : Z){
        ZDDNode* node = pair.second;
        std::cout<<"Node in Z: "<<std::endl;
        printZDD(node);
    }
}

void DancingLinks::printCache(){
    std::cout<<"Cache: "<<std::endl;
    for (const auto& pair : C){
        std::string key = pair.first;
        ZDDNode* node = pair.second;
        std::cout<<"Key: "<<key<<std::endl;
        printZDD(node);
    }
}

void DancingLinks::printColumnHeaders(){
    std::cout<<"Column Headers:"<<std::endl;
    ColumnHeader* current = static_cast<ColumnHeader*>(root->right);
    while (current != root) {
        std::cout<<"Col: "<<current->col<<" Size: "<<current->size<<" ";
        Node* cur = current->down;
        std::cout<<"rows: (";
        do{
            std::cout<<cur->row<<" ";
            cur=cur->down;
        }while(cur!=current);
        std::cout<<")";
        current = static_cast<ColumnHeader*>(current->right);
    }
    std::cout<<std::endl;
}

void DancingLinks::printRowNodes(){
    std::cout<<"Row Nodes: "<<std::endl;
    for (int i = 0; i < ROWS; i++){
        Node* current = RowIndex[i].right;
        if(current != nullptr){
            Node* Note = current;
            std::cout<<"Row: "<<current->row<<" Col: "<<current->col<<" ";
            current = current->right;
            while (current != Note ){
                std::cout<<"Row: "<<current->row<<" Col: "<<current->col<<" ";
                current = current->right;
            }
            std::cout<<std::endl;
        }
        else{
            break;
        }
    }
}

void DancingLinks::printRemainingColumns() {
    if(root->right == root){
        std::cout<<"所有列已覆盖\n";
        return;
    }
    std::cout<<"剩余列及其大小：\n";
    Node* cur = root->right;
    while (cur != root) {
        ColumnHeader* header = static_cast<ColumnHeader*>(cur);
        std::cout<<"列："<<header->col<<"，大小："<<header->size <<" 行：";
        Node* note = cur->down;
        while(note!=cur){
            std::cout<<note->row<<" ";
            note = note->down;
        }
        std::cout<<std::endl;
        cur = cur->right;
    }
    return ;
}