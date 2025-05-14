#include "DancingLinks.h"

/**
 * @brief DancingLinks算法的构造函数
 * @param rows 矩阵的行数
 * @param cols 矩阵的列数
 * @param matrix 输入的01矩阵
 * 
 * 该函数：
 * 1. 初始化DancingLinks数据结构
 * 2. 创建列头节点和行节点
 * 3. 根据输入矩阵构建DancingLinks结构
 */
DancingLinks::DancingLinks( 
    int rows, 
    int cols, 
    const std::vector<std::vector<int>>& matrix
    ) : ROWS(rows), COLS(cols), countNum(0), countSolution(0), timeoutEnabled(false), timeoutSeconds(0){
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

/**
 * @brief DancingLinks算法的析构函数
 * 
 * 该函数：
 * 1. 释放所有动态分配的内存
 * 2. 清理ZDD节点缓存
 * 3. 删除所有节点和数据结构
 */
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

/**
 * @brief 在DancingLinks结构中插入一个节点
 * @param r 行号
 * @param c 列号
 * 
 * 该函数：
 * 1. 在指定行和列的位置插入一个新节点
 * 2. 更新相关的链接关系
 * 3. 维护列的大小计数
 */
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

/**
 * @brief 覆盖（删除）指定列及其相关行
 * @param c 要覆盖的列号
 * 
 * 该函数：
 * 1. 从列头链表中移除指定列
 * 2. 删除该列中所有1所在的行
 * 3. 更新相关列的计数
 */
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

/**
 * @brief 恢复（取消覆盖）指定列及其相关行
 * @param c 要恢复的列号
 * 
 * 该函数：
 * 1. 恢复该列中所有1所在的行
 * 2. 将列重新插入到列头链表中
 * 3. 恢复相关列的计数
 */
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

/**
 * @brief 将当前列状态转换为向量表示
 * @param vec 用于存储列状态的向量
 * 
 * 该函数将当前未被覆盖的列转换为一个布尔向量
 */
void DancingLinks::columnToVector(std::vector<bool>& vec)
{
    ColumnHeader* cur = root;
    while( cur->right != root ){
        vec[cur->right->col - 1] = true;
        cur = (ColumnHeader*)cur->right;
    }
}

/**
 * @brief 比较两个ZDD子树是否相等
 * @param s 第一个ZDD节点
 * @param t 第二个ZDD节点
 * @return 如果子树相等返回true，否则返回false
 */
bool areSubtreesEqual(ZDDNode* s, ZDDNode* t){
    if(!s && !t)return true;

    if(!s || !t)return false;

    if(s->label != t->label)return false;

    if(!areSubtreesEqual(s->lo, t->lo)) return false;

    if(!areSubtreesEqual(s->hi, t->lo)) return false;

    return true;
}

/**
 * @brief 在ZDD树中查找指定的子树
 * @param s 要搜索的ZDD树
 * @param t 要查找的子树
 * @return 如果找到返回子树指针，否则返回nullptr
 */
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

/**
 * @brief 打印ZDD树的结构
 * @param node ZDD树的根节点
 */
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

/**
 * @brief 计算ZDD节点的哈希值
 * @param r 行号
 * @param x 左子节点
 * @param y 右子节点
 * @return 计算得到的哈希值
 */
size_t DancingLinks::hashFunction(int r, ZDDNode* x, ZDDNode* y)
{
    return std::hash<int>()(r) ^ (std::hash<int>()(x->label) << 1) ^ (std::hash<int>()(y->label) << 2);
}

/**
 * @brief 创建或查找唯一的ZDD节点
 * @param r 行号
 * @param x 左子节点
 * @param y 右子节点
 * @return 唯一的ZDD节点指针
 */
ZDDNode* DancingLinks::unique(int r, ZDDNode* x, ZDDNode* y){
    // 增加节点计数，用于统计总共创建的节点数
    countNum++;

    // 计算当前节点的哈希值，用于在Z表中查找
    std::size_t key = hashFunction(r, x, y);
    
    // 检查是否已经存在相同的节点
    if (Z.find(key) == Z.end()) {
        // 获取左右子节点
        ZDDNode* lo = x;
        ZDDNode* hi = y;

        // 如果左右子节点都是终端节点
        if(x->isTerminal && y->isTerminal){
            // 创建新节点并存入Z表
            Z[key] = new ZDDNode(r, lo, hi);
            return Z[key];
        }

        // 如果左右子节点都不是终端节点
        if(!x->isTerminal && !y->isTerminal){
            // 检查当前列状态是否在缓存中
            if (C.find(getColumnState()) != C.end()) {
                // 如果存在，使用缓存中的节点作为右子节点
                hi = C[getColumnState()];
            }

            // 创建新节点并存入Z表
            Z[key] = new ZDDNode(r, lo, hi);
            return Z[key];
        }

        // 处理只有一个子节点是终端节点的情况
        if(x->isTerminal) {
            // 如果左子节点是终端节点，使用F节点
            lo = F;
        } else if (y->isTerminal) {
            // 如果右子节点是终端节点，使用T节点
            hi = T;
        }

        // 创建新节点并存入Z表
        Z[key] = new ZDDNode(r, lo, hi);
    }
    // 返回找到的或新创建的节点
    return Z[key];
}

// ZDDNode* DancingLinks::unique(int r, ZDDNode* lo, ZDDNode* hi) {
//     // ZDD 零抑制规则：如果 hi 分支是 False 节点（⊥），直接返回 lo 节点
//     if (hi == F) {
//         return lo;
//     }

//     // 计算唯一化哈希值
//     std::size_t key = hashFunction(r, lo, hi);

//     // 检查是否已经存在该节点
//     auto it = Z.find(key);
//     if (it != Z.end()) {
//         return it->second;  // 返回已有节点
//     }

//     // 不存在，则创建新节点
//     ZDDNode* node = new ZDDNode(r, lo, hi);
//     Z[key] = node;

//     countNum++;  // 增加唯一节点数统计
//     return node;
// }


/**
 * @brief 获取当前列状态的字符串表示
 * @return 表示列状态的字符串
 */
std::string DancingLinks::getColumnState() const{
    std::string columnState(COLS, '0');
    ColumnHeader* cur = (ColumnHeader*)root->right;
    while (cur != root) {
        columnState[cur->col - 1] = '1';
        cur = (ColumnHeader*)cur->right;
    }
    return columnState;
}

/**
 * @brief 设置超时时间
 * @param seconds 超时时间（秒）
 */
void DancingLinks::setTimeout(double seconds) {
    timeoutSeconds = seconds;
    timeoutEnabled = (seconds > 0);
    resetTimeout();
}

/**
 * @brief 检查是否已超时
 * @return 如果已超时返回true，否则返回false
 */
bool DancingLinks::isTimeout() const {
    if (!timeoutEnabled) return false;
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - startTime).count();
    return elapsed >= timeoutSeconds;
}

/**
 * @brief 重置超时计时器
 */
void DancingLinks::resetTimeout() {
    startTime = std::chrono::high_resolution_clock::now();
}

/**
 * @brief 获取已经过的时间
 * @return 已经过的时间（秒）
 */
double DancingLinks::getElapsedTime() const {
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - startTime).count();
}

/**
 * @brief 使用DancingLinks算法搜索解决方案
 * @return 表示解决方案的ZDD节点
 * 
 * 该函数：
 * 1. 递归搜索所有可能的解决方案
 * 2. 使用缓存优化搜索过程
 * 3. 构建表示所有解决方案的ZDD
 */
ZDDNode* DancingLinks::search()
{
    // 检查是否超时
    if (isTimeout()) {
        std::cout << "计算超时，已停止搜索。已用时间: " << getElapsedTime() << " 秒" << std::endl;
        return F; // 返回假节点，表示未找到解决方案
    }
    
    // 如果根节点的右指针指向自己，说明所有列都已被覆盖，找到一个解决方案
    if(root->right == root){
        countSolution++; // 增加解决方案计数
        return T; // 返回真节点，表示找到一个有效解决方案
    }

    // 获取当前列状态的字符串表示，用于缓存查找
    std::string columnState = getColumnState();

    //检查当前列状态是否已在缓存中，如果在则直接返回缓存的结果
    if (C.find(columnState) != C.end()) {
        countSolution++; // 增加解决方案计数
        return C[columnState]; // 返回缓存中的结果
    }

    // 选择大小最小的列作为当前处理的列，这是DancingLinks算法的关键优化
    ColumnHeader* choose = (ColumnHeader*)root->right;
    ColumnHeader* cur = choose;
    while( cur != root )
    {
        // 选择大小最小的列，减少分支因子
        if( choose->size > cur->size ) choose = cur;
        cur = (ColumnHeader*)cur->right;
    }

    // 如果选择的列大小为0，说明无法继续，返回假节点
    if( choose->size <= 0 ){
        return F;
    }
    
    // 初始化结果为假节点
    ZDDNode* x = F;

    // 覆盖（删除）当前选择的列
    cover(choose->col);
    
    // 遍历当前列中的所有行
    Node* curC = choose->up;
    while( curC != choose )
    {
        // 检查是否超时
        if (isTimeout()) {
            // 恢复（取消覆盖）当前选择的列
            uncover(choose->col);
            return F; // 返回假节点，表示未找到解决方案
        }
        
        //printColumnHeaders();
        Node* noteR = curC;
        Node* curR = curC->right;
        
        // 覆盖当前行中所有1所在的列
        while( curR != noteR )
        {
            cover( curR->col );
            curR = curR->right;
        }
        
        // 递归搜索剩余问题
        ZDDNode* y = search();
        
        // 如果找到解决方案，则更新结果
        if( y->label != -2 ){
            x = unique(curC->row, x, y);
        }
        //printColumnHeaders();
        
        // 恢复当前行中所有1所在的列
        curR = noteR->left;
        while( curR != noteR )
        {
            uncover( curR->col );
            curR = curR->left;
        }
        
        // 移动到当前列中的下一行
        curC = curC->up;
    }
    
    // 恢复（取消覆盖）当前选择的列
    uncover(choose->col);
    
    // 将当前列状态和对应的结果存入缓存
    C[columnState] = x;
    
    // 返回结果
    return x;
}

/**
 * @brief 不使用缓存的DancingLinks搜索函数
 * @return 表示所有解决方案的ZDD节点
 * 
 * 该函数：
 * 1. 递归搜索所有可能的解决方案
 * 2. 不使用缓存，直接构建ZDD
 * 3. 返回表示所有解决方案的ZDD
 */
ZDDNode* DancingLinks::searchWithoutCache()
{
    // 检查是否超时
    if (isTimeout()) {
        std::cout << "计算超时，已停止搜索。已用时间: " << getElapsedTime() << " 秒" << std::endl;
        return F;
    }
    
    // 如果根节点的右指针指向自己，说明所有列都已被覆盖，找到一个解决方案
    if(root->right == root){
        countSolution++;
        return T;
    }

    // 选择大小最小的列作为当前处理的列
    ColumnHeader* choose = (ColumnHeader*)root->right;
    ColumnHeader* cur = choose;
    while( cur != root )
    {
        if( choose->size > cur->size ) choose = cur;
        cur = (ColumnHeader*)cur->right;
    }

    // 如果选择的列大小为0，说明无法继续，返回假节点
    if( choose->size <= 0 ){
        return F;
    }
    
    // 初始化结果为假节点
    ZDDNode* x = F;

    // 覆盖（删除）当前选择的列
    cover(choose->col);
    
    // 遍历当前列中的所有行
    Node* curC = choose->down;
    while( curC != choose )
    {
        // 检查是否超时
        if (isTimeout()) {
            uncover(choose->col);
            return F;
        }
        
        Node* noteR = curC;
        Node* curR = curC->right;
        
        // 覆盖当前行中所有1所在的列
        while( curR != noteR )
        {
            cover( curR->col );
            curR = curR->right;
        }
        
        // 递归搜索剩余问题
        ZDDNode* y = searchWithoutCache();
        
        // 如果找到解决方案，则更新结果
        if( y->label != -2 ){
            x = unique(curC->row, x, y);
        }
        
        // 恢复当前行中所有1所在的列
        curR = noteR->left;
        while( curR != noteR )
        {
            uncover( curR->col );
            curR = curR->left;
        }
        
        // 移动到当前列中的下一行
        curC = curC->down;
    }
    
    // 恢复（取消覆盖）当前选择的列
    uncover(choose->col);
    
    return x;
}

/**
 * @brief 使用DancingLinks算法搜索解决方案，并正确计算缓存中的解数量
 * @return 表示解决方案的ZDD节点
 */
ZDDNode* DancingLinks::searchWithCache() {
    // 检查是否超时
    if (isTimeout()) {
        std::cout << "计算超时，已停止搜索。已用时间: " << getElapsedTime() << " 秒" << std::endl;
        return F;
    }
    
    // 如果根节点的右指针指向自己，说明所有列都已被覆盖，找到一个解决方案
    if(root->right == root){
        countSolution++;
        return T;
    }

    // 获取当前列状态的字符串表示，用于缓存查找
    std::string columnState = getColumnState();

    // 检查当前列状态是否已在缓存中
    if (C.find(columnState) != C.end()) {
        // 获取缓存中的结果
        ZDDNode* cachedResult = C[columnState];
        //如果缓存的结果是有效解，需要计算子矩阵的解数量
        if (cachedResult != F) {
            // 计算子矩阵的解数量
            int subSolutionCount = countSolutionsInZDD(cachedResult);
            // 如果子矩阵有解，则增加解的数量
            if (subSolutionCount > 0) {
                countSolution += subSolutionCount;
            }
        }
        return cachedResult;
    }

    // 选择大小最小的列作为当前处理的列
    ColumnHeader* choose = (ColumnHeader*)root->right;
    ColumnHeader* cur = choose;
    while( cur != root ) {
        if( choose->size > cur->size ) choose = cur;
        cur = (ColumnHeader*)cur->right;
    }

    // 如果选择的列大小为0，说明无法继续，返回假节点
    if( choose->size <= 0 ){
        return F;
    }
    
    // 初始化结果为假节点
    ZDDNode* x = F;

    // 覆盖（删除）当前选择的列
    cover(choose->col);
    
    // 遍历当前列中的所有行
    Node* curC = choose->down;
    while( curC != choose ) {
        // 检查是否超时
        if (isTimeout()) {
            uncover(choose->col);
            return F;
        }
        
        Node* noteR = curC;
        Node* curR = curC->right;
        
        // 覆盖当前行中所有1所在的列
        while( curR != noteR ) {
            cover( curR->col );
            curR = curR->right;
        }
        
        // 递归搜索剩余问题
        ZDDNode* y = searchWithCache();
        
        // 如果找到解决方案，则更新结果
        if( y->label != -2 ){
            x = unique(curC->row, x, y);
        }
        
        // 恢复当前行中所有1所在的列
        curR = noteR->left;
        while( curR != noteR ) {
            uncover( curR->col );
            curR = curR->left;
        }
        
        // 移动到当前列中的下一行
        curC = curC->down;
    }
    
    // 恢复（取消覆盖）当前选择的列
    uncover(choose->col);
    
    // 将当前列状态和对应的结果存入缓存
    C[columnState] = x;
    
    return x;
}

/**
 * @brief 统计ZDD中从根节点到真终端节点T的所有可能路径数
 * @param node ZDD节点
 * @return 路径数量
 */
int DancingLinks::countSolutionsInZDD(ZDDNode* node) {
    if (!node) return 0;
    
    // 如果是终端节点
    if (node->isTerminal) {
        return (node == T) ? 1 : 0;  // 如果是T节点返回1，否则返回0
    }
    
    //如果已经计算过，直接返回缓存的结果
    if (solutionCount.find(node) != solutionCount.end()) {
        return solutionCount[node];
    }
    
    // 计算左子树（0分支）和右子树（1分支）的路径数
    int leftCount = countSolutionsInZDD(node->lo);
    int rightCount = countSolutionsInZDD(node->hi);
    
    // 当前节点的路径数是左右子树路径数之和
    // 因为每个解要么选择当前行（右子树），要么不选择（左子树）
    int totalCount = leftCount + rightCount;
    
    // 缓存结果
    solutionCount[node] = totalCount;
    
    return totalCount;
}


/**
 * @brief 打印ZDD表的内容
 */
void DancingLinks::printTable(){
    std::cout<<"Table:"<<std::endl;
    for (const auto& pair : Z){
        ZDDNode* node = pair.second;
        std::cout<<"Node in Z: "<<std::endl;
        printZDD(node);
    }
}

/**
 * @brief 打印缓存的内容
 */
void DancingLinks::printCache(){
    std::cout<<"Cache: "<<std::endl;
    for (const auto& pair : C){
        std::string key = pair.first;
        ZDDNode* node = pair.second;
        std::cout<<"Key: "<<key<<std::endl;
        printZDD(node);
    }
}

/**
 * @brief 打印列头信息
 */
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

/**
 * @brief 打印行节点信息
 */
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

/**
 * @brief 打印剩余列的信息
 */
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

void DancingLinks::printZDDStructure(ZDDNode* node, int depth, std::string prefix) {
    if (!node) return;
    
    // 打印当前节点
    std::cout << prefix;
    if (node->isTerminal) {
        std::cout << (node->label == -1 ? "T" : "F") << std::endl;
    } else {
        std::cout << "Node(" << node->label << ")" << std::endl;
    }
    
    // 打印左子树（0分支）
    if (node->lo) {
        std::cout << prefix << "├─0─";
        printZDDStructure(node->lo, depth + 1, prefix + "│  ");
    }
    
    // 打印右子树（1分支）
    if (node->hi) {
        std::cout << prefix << "└─1─";
        printZDDStructure(node->hi, depth + 1, prefix + "   ");
    }
}

void DancingLinks::printZDDDotFormat(ZDDNode* node, std::ostream& out) {
    if (!node) return;
    
    static std::set<ZDDNode*> visited;
    if (visited.find(node) != visited.end()) return;
    visited.insert(node);
    
    // 输出节点
    out << "    node" << node << " [label=\"";
    if (node->isTerminal) {
        out << (node->label == -1 ? "T" : "F");
    } else {
        out << node->label;
    }
    out << "\"];\n";
    
    // 输出边和子节点
    if (node->lo) {
        out << "    node" << node << " -> node" << node->lo << " [label=\"0\"];\n";
        printZDDDotFormat(node->lo, out);
    }
    if (node->hi) {
        out << "    node" << node << " -> node" << node->hi << " [label=\"1\"];\n";
        printZDDDotFormat(node->hi, out);
    }
    
    // 如果是根节点，清除访问记录
    if (node == T || node == F) {
        visited.clear();
    }
}