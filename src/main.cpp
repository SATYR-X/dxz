#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>
#include "../include/DancingLinks.h"

namespace fs = std::filesystem;

bool startsWith(const std::string& str, char ch){
    return !str.empty() && str[0] == ch;
}

void extractCR(const std::string& line, int& c, int& r){
    std::istringstream iss(line);
    std::string token;

    iss >> c;
    iss >> r;
}

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
            std::cout<< "该文件名以s开头"<<endl;
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
            std::cout<<"该文件名不以s开头"<<std::endl;
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