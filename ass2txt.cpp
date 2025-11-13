#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <regex>

namespace fs = std::filesystem;

// 检查文件是否为.ass文件
bool isAssFile(const fs::path& path) {
    return path.extension() == ".ass" || path.extension() == ".ASS";
}

// 提取第9个逗号后的字幕文本
std::string extractDialogueText(const std::string& line) {
    const std::string prefix = "Dialogue:";
    if (line.rfind(prefix, 0) != 0) return "";

    std::string content = line.substr(prefix.size());
    int commaCount = 0;
    size_t pos = 0;

    // 找第9个逗号位置
    while (commaCount < 9 && (pos = content.find(',', pos)) != std::string::npos) {
        ++commaCount;
        ++pos;
    }

    if (commaCount < 9 || pos >= content.size()) return "";
    return content.substr(pos);
}

// 去掉ASS标签 {xxx} 以及多余空格
std::string cleanAssTags(std::string text) {
    static const std::regex tagPattern(R"(\{[^}]*\})");  // 匹配 { ... }
    text = std::regex_replace(text, tagPattern, "");     // 删除所有标签

    // 去掉多余空格
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t'))
        text.erase(text.begin());
    while (!text.empty() && (text.back() == ' ' || text.back() == '\r' || text.back() == '\n'))
        text.pop_back();

    return text;
}

// 处理单个.ass文件
void convertAssToText(const fs::path& assFile, const fs::path& outputFile) {
    std::ifstream inFile(assFile);
    if (!inFile.is_open()) {
        std::cerr << "无法打开文件: " << assFile << std::endl;
        return;
    }

    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "无法创建输出文件: " << outputFile << std::endl;
        return;
    }

    std::string line;
    bool inDialogueSection = false;

    while (std::getline(inFile, line)) {
        if (line.find("[Events]") != std::string::npos) {
            inDialogueSection = true;
            continue;
        }

        if (inDialogueSection && line.rfind("Dialogue:", 0) == 0) {
            std::string text = extractDialogueText(line);
            text = cleanAssTags(text);
            if (!text.empty())
                outFile << text << std::endl;
        }
    }

    std::cout << "转换完成: " << assFile.filename() << " -> " << outputFile.filename() << std::endl;
}

// 递归处理所有子文件夹
void processFolder(const fs::path& dir) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && isAssFile(entry.path())) {
            fs::path outputFile = entry.path();
            outputFile.replace_extension(".txt");
            convertAssToText(entry.path(), outputFile);
        }
    }
}

int main() {
    fs::path dir = "./";  // 当前目录
    std::cout << "正在递归遍历文件夹: " << dir << std::endl;

    processFolder(dir);

    std::cout << "全部转换完成！" << std::endl;
    return 0;
}
