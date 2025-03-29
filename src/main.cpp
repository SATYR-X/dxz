#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>

namespace fs = std::filesystem;

int main() {
    
    try {
        const std::string folderPath1 = "../exact_cover_benchmark";
        const std::string folderPath2 = "../set_partitioning_benchmarks";

        for(const auto& entry : fs::directory_iterator(folderPath2)) {
            if(entry.is_regular_file() && entry.path().extension() == ".txt") {
                int r;
                int c;

                std::vector<std::vector<int>> X = proFileToMat(entry.path(), r, c);
                
            }
        }
    }
    
    return 0;
}