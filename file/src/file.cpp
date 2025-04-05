#include <fstream>
#include <filesystem>

#include "DCfile.h"
#include "MD5.h"

namespace DC {
    //构造函数===============================================================================================
//手动传入路径、名称、扩展名和内容--------------------------------------------------------------
//接收二进制文件
    file::file(std::string ext, std::vector<char> content, std::string path, std::string name) :
        path(std::move(path)),
        name(std::move(name)),
        ext(std::move(ext)),
        DataValid(false),
        PathValid(false)
    {
        data = std::make_shared<std::vector<char>>(content);
        DataValid = !data->empty();
        validatePath();
    }
    //接收字符串文件
    file::file(std::string ext, std::string content, std::string path, std::string name) :
        path(std::move(path)),
        name(std::move(name)),
        ext(std::move(ext)),
        DataValid(false),
        PathValid(false)
    {
        std::vector<char> contentVec(content.begin(), content.end());
        data = std::make_shared<std::vector<char>>(contentVec);
        DataValid = !data->empty();
        validatePath();
    }
    //传入路径，从文件系统中获取--------------------------------------------------------------------
    file::file(const std::string& filePath) :
        path(filePath),
        DataValid(false),
        PathValid(false)
    {
        // 使用 std::filesystem::path 来处理文件路径
        std::filesystem::path fsPath(filePath);
        name = fsPath.stem().string();
        ext = fsPath.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        validatePath();
        if (PathValid) {
            try {
                data = std::make_shared<std::vector<char>>(readFileData());
                DataValid = true;
            }
            catch (...) {
                DataValid = false;
            }
        }
    }
    //===============================================================================================构造函数
    //元数据获取=============================================================================================
    // 获取文件的最后修改时间并格式化为 YYYY-MM-DD hh:mm:ss
    //std::string file::getTime() const {
    //    // 转换文件时间为系统时间
    //    auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(lastModifiedTime);
    //    std::time_t cftime = std::chrono::system_clock::to_time_t(systemTime);

    //    // 将时间格式化为 YYYY-MM-DD hh:mm:ss
    //    std::stringstream ss;
    //    ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
    //    return ss.str();
    //}
    // 获取文件的大小
    size_t file::size() const {
        if (!DataValid) {
            // 延迟加载文件数据
            auto mutableThis = const_cast<file*>(this);
            mutableThis->getData();
        }
        return data->size();
    }
    // 获取完整的文件名（含扩展名）
    std::string file::fullName() const {
        return std::string(name) + (ext.empty() ? "" : "." + std::string(ext));
    }
    std::string file::getName() const {
        return name;
    }
    std::string file::getExt() const {
        return ext;
    }
    // 验证路径是否有效
    void file::validatePath() {
        try {
            if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
                PathValid = true;
            }
            else {
                PathValid = false;
            }
        }
        catch (...) {
            PathValid = false;
        }
    }
    // 判断文件数据是否有效
    bool file::isDataValid() const {
        return DataValid;
    }
    // 判断文件路径是否有效
    bool file::isPathValid() const {
        return PathValid;
    }
    std::string file::getMd5(){
        // 如果已经计算过 MD5，则直接返回缓存的结果
        if (!md5.empty()) {
            return md5;
        }
        try {
            MD5 md5Calculator; // 创建 MD5 类的实例
            // 判断文件内容是否已经加载，如果未加载则加载数据
            if (!DataValid) {
                getData();
            }
            // 获取文件数据并计算 MD5
            md5Calculator.update(data->data(), data->size()); // 使用 MD5 类更新每一行的数据
            md5Calculator.finalize();
            md5 = md5Calculator.hexdigest(); // 获取十六进制摘要
            std::transform(md5.begin(), md5.end(), md5.begin(), ::toupper); // 将 MD5 结果转为大写
            return md5;
        }
        catch (...) {
            return "";
        }
    }
    //=============================================================================================元数据获取
    //内容获取===============================================================================================
    // 接口：读取二进制内容-------------------------------------------------------------
    const std::vector<char> file::getData(){
        if (!DataValid) {
            data = std::make_shared<std::vector<char>>(readFileData());
        }
        return *data;
    }
    // 接口：读取文本内容---------------------------------------------------------------
    const std::string file::getText() {
        if (!DataValid) {
            getData();  // 延迟加载数据
        }
        // 执行行拆分和换行符处理
        return assembleText(*data);
    }
    // 加载文件到实例
    const std::vector<char> file::readFileData() {
        std::ifstream fileStream(path, std::ios::binary | std::ios::ate);
        if (!fileStream) {
            DataValid = false;
            PathValid = false;
            return {};
        }
        auto size = fileStream.tellg();
        std::vector<char> buffer(size);
        fileStream.seekg(0, std::ios::beg);
        fileStream.read(buffer.data(), size);
        DataValid = true;
        return buffer;
    }
    //===============================================================================================内容获取
    // 辅助方法：将文件内容按行分割
    const std::vector<std::vector<char>> file::splitLines(const std::vector<char>& fileContent)const {
        std::vector<std::vector<char>> lines;
        // 假设行数大约为1024，行宽大约为256，可以根据实际情况调整
        size_t estimatedLineCount = 1024;
        size_t estimatedLineLength = 512;
        lines.reserve(estimatedLineCount);  // 预分配行容器的空间
        std::vector<size_t> lineEndPositions;
        // 记录所有换行符的位置
        for (size_t i = 0; i < fileContent.size(); ++i) {
            if (fileContent[i] == '\n') {
                lineEndPositions.push_back(i);
            }
        }
        // 分配每一行的空间并将其分割
        size_t start = 0;
        for (size_t end : lineEndPositions) {
            // 估算每行的长度并预分配空间
            size_t lineLength = end - start + 1;
            std::vector<char> line;
            line.reserve(lineLength);  // 为每行分配空间
            line.insert(line.end(), fileContent.begin() + start, fileContent.begin() + end + 1);
            lines.push_back(std::move(line));
            start = end + 1;
        }
        // 处理没有以换行符结尾的最后一行
        if (start < fileContent.size()) {
            std::vector<char> line;
            line.reserve(fileContent.size() - start);  // 最后一行
            line.insert(line.end(), fileContent.begin() + start, fileContent.end());
            lines.push_back(std::move(line));
        }
        return lines;
    }

    // 辅助方法：将按行分割的数据重新组装为二进制内容
    const std::vector<char> file::assembleBinary(const std::vector<std::vector<char>>& lines) const {
        std::vector<char> binaryData;
        binaryData.reserve(size());
        for (const auto& line : lines) {
            binaryData.insert(binaryData.end(), line.begin(), line.end());
        }
        return binaryData;
    }
    // 辅助方法：将按行分割的数据重新组装为文本内容，处理换行符
    const std::string file::assembleText(const std::vector<char>& data) const {
        std::string text;
        size_t i = 0;
        while (i < data.size()) {
            if (data[i] == '\n') {
                if (i > 0 && data[i - 1] == '\r') {
                    text.pop_back();  // 去除 '\r'，保留 '\n'
                    text.push_back('\n');
                }
            }
            else {
                text.push_back(data[i]);
            }
            ++i;
        }
        return text;
    }
}