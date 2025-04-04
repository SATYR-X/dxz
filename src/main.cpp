#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>
#include "../include/DancingLinks.h"

namespace fs = std::filesystem;

/**
 * @brief 检查字符串是否以特定字符开头
 * @param str 要检查的输入字符串
 * @param ch 要检查的字符
 * @return 如果字符串以该字符开头返回true，否则返回false
 */
bool startsWith(const std::string& str, char ch){
    return !str.empty() && str[0] == ch;
}

/**
 * @brief 从字符串行中提取列数和行数
 * @param line 包含列数和行数的输入字符串
 * @param c 用于存储提取的列数的引用
 * @param r 用于存储提取的行数的引用
 */
void extractCR(const std::string& line, int& c, int& r){
    std::istringstream iss(line);
    std::string token;

    iss >> c;
    iss >> r;
}

/**
 * @brief 将问题文件转换为矩阵表示
 * @param filename 输入文件的路径
 * @param r 用于存储矩阵行数的引用
 * @param c 用于存储矩阵列数的引用
 * @return 表示矩阵的二维向量，1表示存在，0表示不存在
 * @throws std::runtime_error 如果无法打开文件
 */
std::vector<std::vector<int>> proFileToMat(
    const fs::path& filename, int& r, int& c){
        std::ifstream file(filename.string());
        if (!file.is_open()){
            throw std::runtime_error("无法打开文件");
        }

        std::string line;
        std::getline(file, line);

        int n, m;
        extractCR(line, n, m);
        r = m;
        c = n;
        
        std::vector<std::vector<int>> matrix(r, std::vector<int>(c, 0));

        fs::path fileName = filename.filename();
        if(startsWith(filename.string(), 's')){
            int row = 0;
            while (std::getline(file, line)){
                std::istringstream iss(line);
                int col;
                while (iss >> col){
                    if(col > 0 && col <= n) {
                        matrix[row][col - 1] = 1;
                    }
                }
                ++row;
            }
            file.close();
        }else{
            int row = 0;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string token;
                iss >> token;
                iss >> token;
                int col;
                while(iss >> col){
                    if(col > 0 && col <= n){
                        matrix[row][col - 1] = 1;
                    }
                }
                ++row;
            }
            file.close();
        }
        return matrix;
    }

/**
 * @brief 主函数，处理精确覆盖和集合划分的基准测试文件
 * @return 成功执行返回0
 * 
 * 该函数的功能：
 * 1. 遍历指定目录中的基准测试文件
 * 2. 将每个文件转换为矩阵表示
 * 3. 使用DancingLinks算法解决精确覆盖问题
 * 4. 测量并报告计算时间和解决方案统计信息
 */
int main() {
    
    try {
        const std::string folderPath1 = "../exact_cover_benchmark";
        const std::string folderPath2 = "../set_partitioning_benchmarks";

        for(const auto& entry : fs::directory_iterator(folderPath2)) {
            if(entry.is_regular_file() && entry.path().extension() == ".txt") {
                int r;
                int c;

                std::vector<std::vector<int>> X = proFileToMat(entry.path(), r, c);
                
                {

                DancingLinks dlm(r, c, X);
                //dlm.printColumnHeaders();
                
                auto start = std::chrono::high_resolution_clock::now();
                ZDDNode* z = dlm.search();
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

                std::cout<<"当前文件："<<entry.path().filename().string()<<std::endl;
                std::cout<<"Search Compulation Time: "<<duration.count()<<" seconds.\n";


                std::cout<<"节点数量："<<dlm.countNum<<std::endl;

                std::cout<<"solution nums: "<<dlm.countSolution<<std::endl;
                
                // std::vector<std::string> paths = binaryTreePaths(z);
                // for ( const auto& path : paths ) {
                //     std::cout<<path<<std::endl;
                // }

                std::cout<<std::endl;
                }
            }
        }
    } catch (const std::exception& e){
        std::cerr << "错误： " << e.what() << std::endl;
    }
    
    return 0;
}