#pragma once
#include <string>
#include <vector>
#include <memory>

namespace DC {
    class file {
    public:
        //构造函数==========================================================================
        file() : DataValid(false), PathValid(false) {}
        file(
            std::string ext,                // 扩展名
            std::vector<char> content,      // 内容
            std::string path = "",          // 路径，可省略
            std::string name = ""           // 文件名，可省略
        );
        file(
            std::string ext,                // 扩展名
            std::string content,            // 内容
            std::string path = "",          // 路径，可省略
            std::string name = ""           // 文件名，可省略
        );
        file(
            const std::string& filePath     // 路径
        );
        //==========================================================================构造函数
        //接口方法==========================================================================
        size_t size() const;                // 获取文件大小
        std::string fullName() const;       // 获取完整的文件名（含扩展名）
        std::string getName() const;        // 获取文件名
        std::string getExt() const;         // 获取扩展名
        // std::string getTime() const;        // 获取修改时间
        const std::vector<char> getData();  // 获取二进制文件内容
        const std::string getText();        // 获取文本文件内容
        std::string getMd5();

        bool isDataValid() const;           // 判断文件内容是否有效
        bool isPathValid() const;           // 判断文件路径是否有效
        //==========================================================================接口方法
    private:
        std::string path;  // 文件路径
        std::string name;  // 文件名
        std::string ext;   // 扩展名
        std::string md5;   // MD5码
        std::shared_ptr<std::vector<char>> data;  // 文件内容
        // std::filesystem::file_time_type lastModifiedTime; // 时间戳

        bool DataValid; // 文件内容是否有效
        bool PathValid; // 文件路径是否有效

        void validatePath();
        const std::vector<std::vector<char>> splitLines(const std::vector<char>& fileContent)const;
        const std::vector<char> assembleBinary(const std::vector<std::vector<char>>& lines)const;
        const std::string assembleText(const std::vector<char>& data)const;
        const std::vector<char> readFileData();
    };
}