#include "../include/DancingLinks.h"

//输出ZDD路径
void findPaths(ZDDNode* node, std::string path, std::vector<std::string>& paths) {
    if (!node) return; // 如果节点为空，直接返回

    // 将当前节点的值添加到路径中
    path += std::to_string(node->label);

    // 如果是叶子节点，保存路径
    if (node->isTerminal) {
        if(node->label == -1){ //T
            paths.push_back(path);
        }
        return;
    } else {
        // 递归遍历左子树
        findPaths(node->lo, path + " -> lo: ", paths);
        // 递归遍历右子树
        findPaths(node->hi, path + " -> hi: ", paths);
    }
}

std::vector<std::string> binaryTreePaths(ZDDNode* root) {
    std::vector<std::string> paths;
    findPaths(root, "", paths);
    return paths;
}


int main(){
    std::vector<std::vector<int>> X = {
                    {1, 1, 1, 0, 1, 0},
                    {1, 1, 0, 0, 0, 0},
                    {0, 0, 0, 1, 0, 1},
                    {0, 0, 1, 1, 0, 1},
                    {0, 0, 1, 0, 1, 0},
                };

    DancingLinks dlm(X.size(), X[0].size(), X);
    dlm.printColumnHeaders();
    //dlm.printRowNodes();


    ZDDNode* z = dlm.search(); 

     //统计ZDD节点个数
    std::cout << "ZDD节点数量: " << dlm.countNum << std::endl;
    
    //统计解的个数      
    std::cout << "解的个数: " << dlm.countSolution << std::endl;

    //层序遍历
    //dlm.printZDD(z);

    // dlm.printTable();
    // dlm.printCache();
    //std::vector<std::string> paths = binaryTreePaths(z);
    //std::cout<<"solution nums: "<< dlm.countSolution<<std::endl;
    // for (const auto& path : paths) {
    //     std::cout << path << std::endl;
    // }
}