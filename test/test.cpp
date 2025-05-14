#include "../include/DancingLinks.h"
#include <fstream>

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
    //dlm.printColumnHeaders();
    //dlm.printRowNodes();

    ZDDNode* z = dlm.search(); 
    
    //统计解的个数      
    std::cout << "\n解的个数: " << dlm.countSolution << std::endl;

    std::cout<<"输出ZDD信息：";

    // 生成dot文件
    std::ofstream out("zdd.dot");
    if (!out.is_open()) {
        std::cerr << "无法创建文件: zdd.dot" << std::endl;
        return 1;
    }

    // 写入DOT文件头
    out << "digraph ZDD {\n";
    out << "    rankdir=TB;\n";
    out << "    node [shape=circle];\n";

    // 输出ZDD结构
    dlm.printZDDDotFormat(z, out);

    // 写入DOT文件尾
    out << "}\n";
    out.close();
    std::cout << "已生成ZDD的dot文件: zdd.dot" << std::endl;

    //层序遍历
    //dlm.printZDD(z);

    // dlm.printTable();
    // dlm.printCache();
    // std::vector<std::string> paths = binaryTreePaths(z);
    // std::cout<<"solution nums: "<< dlm.countSolution<<std::endl;
    // for (const auto& path : paths) {
    //     std::cout << path << std::endl;
    // }
    return 0;
}