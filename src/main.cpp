#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
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
    std::vector<int> nums;
    while (iss >> token) {
        try {
            int val = std::stoi(token);
            nums.push_back(val);
        } catch (...) {}
    }
    if (nums.size() >= 2) {
        c = nums[0];
        r = nums[1];
    } else if (nums.size() == 1) {
        c = nums[0];
        r = -1; // 标记未获取到行数
    } else {
        c = r = -1;
    }
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
    // 自动识别格式
    bool isSetPartitioning = false;
    // 判断首行是否为两个数字
    {
        std::istringstream iss(line);
        int a, b;
        if ((iss >> a) && (iss >> b)) {
            std::string rest;
            std::getline(iss, rest);
            if (rest.empty() || rest.find_first_not_of(' ') == std::string::npos) {
                isSetPartitioning = true;
            }
        }
    }
    if (isSetPartitioning) {
        // set_partitioning_benchmarks格式
        std::istringstream iss(line);
        iss >> c >> r;
        std::cout << "读取文件 " << filename << "，列数: " << c << "，行数: " << r << std::endl;
        std::vector<std::vector<int>> matrix(r, std::vector<int>(c, 0));
        int row = 0;
        while (std::getline(file, line) && row < r) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            int weight, count;
            iss >> weight >> count;
            for (int i = 0; i < count; ++i) {
                int col;
                iss >> col;
                if (col > 0 && col <= c) {
                    matrix[row][col - 1] = 1;
                }
            }
            ++row;
        }
        file.close();
        return matrix;
    } else {
        // exact_cover_benchmark格式
        // 先尝试从首行和后续行提取列数和行数
        int n = -1, m = -1;
        extractCR(line, n, m);
        // 处理只提取到一个数字的情况
        if (m == -1) {
            // 记录当前位置
            std::streampos pos = file.tellg();
            int rowCount = 0;
            while (std::getline(file, line)) {
                if (!line.empty() && line[0] != 'c' && line[0] != 'p') rowCount++;
            }
            m = rowCount;
            // 恢复文件指针到数据起始行
            file.clear();
            file.seekg(pos);
        }
        r = m;
        c = n;
        if (r <= 0 || c <= 0 || r > 200000 || c > 10000) {
            throw std::runtime_error("文件格式错误或矩阵规模异常: 列数=" + std::to_string(c) + ", 行数=" + std::to_string(r));
        }
        std::cout << "读取文件 " << filename << "，列数: " << c << "，行数: " << r << std::endl;
        std::vector<std::vector<int>> matrix(r, std::vector<int>(c, 0));
        int row = 0;
        // 跳过注释和参数行，处理数据行
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == 'c' || line[0] == 'p') continue;
            std::istringstream iss(line);
            std::string token;
            iss >> token; // 跳过's'
            int col;
            while (iss >> col) {
                if (col > 0 && col <= n) {
                    matrix[row][col - 1] = 1;
                }
            }
            ++row;
        }
        file.close();
        return matrix;
    }
}

/**
 * @brief 将新数据集文件转换为矩阵表示
 * @param filename 输入文件的路径
 * @param r 用于存储矩阵行数的引用
 * @param c 用于存储矩阵列数的引用
 * @return 表示矩阵的二维向量，1表示存在，0表示不存在
 * @throws std::runtime_error 如果无法打开文件
 */
std::vector<std::vector<int>> convertNewDatasetFileToMatrix(
    const fs::path& filename, int& r, int& c) {
    std::ifstream file(filename.string());
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件");
    }

    std::string line;
    std::getline(file, line);
    
    // 第一行包含列数和行数
    std::istringstream iss(line);
    iss >> c >> r;
    
    // 初始化矩阵
    std::vector<std::vector<int>> matrix(r, std::vector<int>(c, 0));
    
    // 读取每一行
    int row = 0;
    while (std::getline(file, line) && row < r) {
        std::istringstream iss(line);
        int count;
        iss >> count; // 读取该行中1的个数
        
        // 读取该行中1的列索引
        for (int i = 0; i < count; i++) {
            int col;
            iss >> col;
            if (col > 0 && col <= c) {
                matrix[row][col - 1] = 1;
            }
        }
        row++;
    }
    
    file.close();
    return matrix;
}

/**
 * @brief 处理单个文件
 * @param filePath 文件路径
 * @param showZDD 是否显示ZDD详细信息
 * @param showMatrix 是否显示矩阵
 * @param showTime 是否显示时间
 * @param showSolution 是否显示解决方案
 * @param outputFile 输出文件流
 */
void processFile(const fs::path& filePath, bool showZDD, bool showMatrix, 
                bool showTime, bool showSolution, std::ofstream& outputFile) {
    try {
        int r, c;
        std::vector<std::vector<int>> matrix;
        
        // 检查文件类型
        std::string fileName = filePath.filename().string();
        std::string filePathStr = filePath.string();
        
        // 判断是否为新数据集
        if (filePathStr.find("dataset_new") != std::string::npos || 
            fileName.find("grafo") == 0) {
            matrix = proFileToMat(filePath, r, c);
            
            // 统计矩阵中"1"的数量
            int oneCount = 0;
            for (const auto& rowVec : matrix) {
                for (int val : rowVec) {
                    if (val == 1) ++oneCount;
                }
            }
            std::cout << "矩阵中1的数量: " << oneCount << std::endl;
        } else {
            matrix = proFileToMat(filePath, r, c);
        }
        
        // 统计矩阵中"1"的数量
        int oneCount = 0;
        for (const auto& rowVec : matrix) {
            for (int val : rowVec) {
                if (val == 1) ++oneCount;
            }
        }
        std::cout << "矩阵中1的数量: " << oneCount << std::endl;
        
        // 创建DancingLinks对象
        DancingLinks dlm(r, c, matrix);
        // 设置超时时间为600秒
        dlm.setTimeout(600);
        // 记录开始时间
        auto start = std::chrono::high_resolution_clock::now();
        
        // 搜索解决方案
        //ZDDNode* z = dlm.search();
        //ZDDNode* z = dlm.searchWithoutCache();

        ZDDNode* z = dlm.searchWithCache();

        // 记录结束时间
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        
        // 输出结果
        std::string result = "文件: " + filePath.filename().string() + "\n";
        
        // 判断是否超时
        bool timeout = dlm.getElapsedTime() >= 600.0;
        if (showTime) {
            if (timeout) {
                result += "计算时间: >600 秒\n";
            } else {
                result += "计算时间: " + std::to_string(duration.count()) + " 秒\n";
            }
        }
        
        result += "节点数量: " + std::to_string(dlm.countNum) + "\n";
        result += "搜到根节点的解决方案数量: " + std::to_string(dlm.countSolution) + "\n";
        // 计算ZDD中的解数量
        // int solutionCount = dlm.countSolutionsInZDD(z);
        // result += "直接搜索ZDD中的解决方案数量: " + std::to_string(solutionCount) + "\n";
        
        // 输出ZDD节点数量除以矩阵中1的数量（所有数据集类型都输出）
        if (oneCount > 0) {
            double ratio = static_cast<double>(dlm.countNum) / oneCount;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << ratio;
            result += "节点数/1的数量: " + oss.str() + "\n";
        }
        
        if (showMatrix) {
            result += "矩阵:\n";
            for (const auto& row : matrix) {
                for (int val : row) {
                    result += std::to_string(val) + " ";
                }
                result += "\n";
            }
        }
        
        if (showZDD) {
            result += "ZDD结构:\n";
            dlm.printZDDStructure(z);
        }
        
        if (showSolution) {
            // 这里可以添加显示解决方案的代码
            result += "解决方案已找到\n";
        }
        
        result += "\n";
        
        // 输出结果
        if (outputFile.is_open()) {
            outputFile << result;
        } else {
            std::cout << result;
        }
        
    } catch (const std::exception& e) {
        std::string error = "处理文件 " + filePath.string() + " 时出错: " + e.what() + "\n";
        if (outputFile.is_open()) {
            outputFile << error;
        } else {
            std::cerr << error;
        }
    }
}

/**
 * @brief 获取目录中的文件
 * @param dirPath 目录路径
 * @param extension 文件扩展名
 * @return 文件路径列表
 */
std::vector<fs::path> getFilesInDirectory(const fs::path& dirPath, const std::string& extension) {
    std::vector<fs::path> files;
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return files;
    }
    
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path());
        }
    }
    
    return files;
}

/**
 * @brief 运行基准测试
 * @param exactCoverPath 精确覆盖数据集路径
 * @param setPartitioningPath 集合划分数据集路径
 * @param newDatasetPath 新数据集路径
 * @param outputFile 输出文件流
 */
void runBenchmark(const fs::path& exactCoverPath, const fs::path& setPartitioningPath, 
                 const fs::path& newDatasetPath, std::ofstream& outputFile) {
    // 获取数据集中的文件
    std::vector<fs::path> exactCoverFiles = getFilesInDirectory(exactCoverPath, ".txt");
    std::vector<fs::path> setPartitioningFiles = getFilesInDirectory(setPartitioningPath, ".txt");
    std::vector<fs::path> newDatasetFiles = getFilesInDirectory(newDatasetPath, ".txt");
    
    // 按文件名排序
    std::sort(exactCoverFiles.begin(), exactCoverFiles.end());
    std::sort(setPartitioningFiles.begin(), setPartitioningFiles.end());
    std::sort(newDatasetFiles.begin(), newDatasetFiles.end());
    
    // 输出基准测试头部
    std::string header = "基准测试结果\n";
    header += "=============\n\n";
    
    if (outputFile.is_open()) {
        outputFile << header;
    } else {
        std::cout << header;
    }
    
    // 处理精确覆盖数据集
    std::string exactCoverHeader = "精确覆盖数据集 (" + exactCoverPath.string() + "):\n";
    exactCoverHeader += "----------------------------------------\n";
    
    if (outputFile.is_open()) {
        outputFile << exactCoverHeader;
    } else {
        std::cout << exactCoverHeader;
    }
    
    for (const auto& filePath : exactCoverFiles) {
        processFile(filePath, false, false, true, false, outputFile);
    }
    
    // 处理集合划分数据集
    std::string setPartitioningHeader = "\n集合划分数据集 (" + setPartitioningPath.string() + "):\n";
    setPartitioningHeader += "----------------------------------------\n";
    
    if (outputFile.is_open()) {
        outputFile << setPartitioningHeader;
    } else {
        std::cout << setPartitioningHeader;
    }
    
    for (const auto& filePath : setPartitioningFiles) {
        processFile(filePath, false, false, true, false, outputFile);
    }
    
    // 处理新数据集
    std::string newDatasetHeader = "\n新数据集 (" + newDatasetPath.string() + "):\n";
    newDatasetHeader += "----------------------------------------\n";
    
    if (outputFile.is_open()) {
        outputFile << newDatasetHeader;
    } else {
        std::cout << newDatasetHeader;
    }
    
    for (const auto& filePath : newDatasetFiles) {
        processFile(filePath, false, false, true, false, outputFile);
    }
}

/**
 * @brief 显示帮助信息
 */
void printHelp() {
    std::cout << "Dancing Links with ZDD 程序\n";
    std::cout << "==========================\n\n";
    std::cout << "功能说明:\n";
    std::cout << "  1. 处理精确覆盖数据集 - 处理精确覆盖问题数据集\n";
    std::cout << "  2. 处理集合划分数据集 - 处理集合划分问题数据集\n";
    std::cout << "  3. 处理新数据集 - 处理新格式的数据集\n";
    std::cout << "  4. 处理单个文件 - 处理指定的单个文件\n";
    std::cout << "  5. 运行基准测试 - 对所有数据集运行基准测试\n";
    std::cout << "  6. 显示帮助信息 - 显示此帮助信息\n";
    std::cout << "  0. 退出程序 - 退出程序\n\n";
    std::cout << "使用说明:\n";
    std::cout << "  1. 选择相应的功能选项\n";
    std::cout << "  2. 按照提示输入必要的参数\n";
    std::cout << "  3. 查看处理结果\n";
}

/**
 * @brief 显示菜单
 */
void showMenu() {
    std::cout << "\nDancing Links with ZDD 程序\n";
    std::cout << "==========================\n\n";
    std::cout << "请选择操作:\n";
    std::cout << "1. 处理精确覆盖数据集\n";
    std::cout << "2. 处理集合划分数据集\n";
    std::cout << "3. 处理新数据集\n";
    std::cout << "4. 处理单个文件\n";
    std::cout << "5. 运行基准测试\n";
    std::cout << "6. 显示帮助信息\n";
    std::cout << "0. 退出程序\n";
    std::cout << "请输入选项 (0-6): ";
}

/**
 * @brief 处理精确覆盖数据集
 */
void processExactCoverDataset() {
    std::string datasetPath = "../exact_cover_benchmark";
    std::vector<fs::path> files = getFilesInDirectory(datasetPath, ".txt");
    if (files.empty()) {
        std::cout << "未找到文件或目录不存在: " << datasetPath << std::endl;
        return;
    }
    std::sort(files.begin(), files.end());
    std::cout << "找到 " << files.size() << " 个文件:\n";
    for (size_t i = 0; i < files.size(); i++) {
        std::cout << i + 1 << ". " << files[i].filename().string() << std::endl;
    }
    std::cout << "\n请选择要处理的文件 (1-" << files.size() << ")，或输入 0 处理所有文件: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    bool showZDD = false;
    bool showMatrix = false;
    std::ofstream outputFile; // 不打开文件，全部输出到控制台
    if (choice == 0) {
        for (const auto& filePath : files) {
            processFile(filePath, showZDD, showMatrix, true, false, outputFile);
        }
    } else if (choice >= 1 && choice <= static_cast<int>(files.size())) {
        processFile(files[choice - 1], showZDD, showMatrix, true, false, outputFile);
    } else {
        std::cout << "无效的选择\n";
    }
}

/**
 * @brief 处理集合划分数据集
 */
void processSetPartitioningDataset() {
    std::string datasetPath = "../set_partitioning_benchmarks";
    std::vector<fs::path> files = getFilesInDirectory(datasetPath, ".txt");
    if (files.empty()) {
        std::cout << "未找到文件或目录不存在: " << datasetPath << std::endl;
        return;
    }
    std::sort(files.begin(), files.end());
    std::cout << "找到 " << files.size() << " 个文件:\n";
    for (size_t i = 0; i < files.size(); i++) {
        std::cout << i + 1 << ". " << files[i].filename().string() << std::endl;
    }
    std::cout << "\n请选择要处理的文件 (1-" << files.size() << ")，或输入 0 处理所有文件: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    bool showZDD = false;
    bool showMatrix = false;
    std::ofstream outputFile;
    if (choice == 0) {
        for (const auto& filePath : files) {
            processFile(filePath, showZDD, showMatrix, true, false, outputFile);
        }
    } else if (choice >= 1 && choice <= static_cast<int>(files.size())) {
        processFile(files[choice - 1], showZDD, showMatrix, true, false, outputFile);
    } else {
        std::cout << "无效的选择\n";
    }
}

/**
 * @brief 处理新数据集
 */
void processNewDataset() {
    std::string datasetPath = "../dataset_new";
    std::vector<fs::path> files = getFilesInDirectory(datasetPath, ".txt");
    if (files.empty()) {
        std::cout << "未找到文件或目录不存在: " << datasetPath << std::endl;
        return;
    }
    std::sort(files.begin(), files.end());
    std::cout << "找到 " << files.size() << " 个文件:\n";
    for (size_t i = 0; i < files.size(); i++) {
        std::cout << i + 1 << ". " << files[i].filename().string() << std::endl;
    }
    std::cout << "\n请选择要处理的文件 (1-" << files.size() << ")，或输入 0 处理所有文件: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    bool showZDD = false;
    bool showMatrix = false;
    std::ofstream outputFile;
    if (choice == 0) {
        for (const auto& filePath : files) {
            processFile(filePath, showZDD, showMatrix, true, false, outputFile);
        }
    } else if (choice >= 1 && choice <= static_cast<int>(files.size())) {
        processFile(files[choice - 1], showZDD, showMatrix, true, false, outputFile);
    } else {
        std::cout << "无效的选择\n";
    }
}

/**
 * @brief 处理单个文件
 */
void processSingleFile() {
    std::string filePath;
    std::cout << "请输入文件路径: ";
    std::getline(std::cin, filePath);
    
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        std::cout << "文件不存在: " << filePath << std::endl;
        return;
    }
    
    bool showZDD = false;
    bool showMatrix = false;
    std::ofstream outputFile;
    processFile(filePath, showZDD, showMatrix, true, false, outputFile);
}

/**
 * @brief 运行基准测试
 */
void runBenchmarkMode() {
    std::string exactCoverPath = "../exact_cover_benchmark";
    std::string setPartitioningPath = "../set_partitioning_benchmarks";
    std::string newDatasetPath = "../dataset_new";
    std::ofstream outputFile;
    runBenchmark(exactCoverPath, setPartitioningPath, newDatasetPath, outputFile);
}

/**
 * @brief 主函数
 * @return 成功执行返回0
 */
int main() {
    std::cout << "欢迎使用 Dancing Links with ZDD 程序\n";
    std::cout << "==========================\n\n";
    
    while (true) {
        showMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 0:
                std::cout << "退出程序\n";
                return 0;
            case 1:
                processExactCoverDataset();
                break;
            case 2:
                processSetPartitioningDataset();
                break;
            case 3:
                processNewDataset();
                break;
            case 4:
                processSingleFile();
                break;
            case 5:
                runBenchmarkMode();
                break;
            case 6:
                printHelp();
                break;
            default:
                std::cout << "无效的选择，请重新输入\n";
                break;
        }
        
        std::cout << "\n按Enter键继续...";
        std::cin.get();
    }
    
    return 0;
}