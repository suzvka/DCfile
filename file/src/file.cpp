#include <fstream>
#include <filesystem>

#include "DCfile.h"
#include "MD5.h"

namespace DC {
    //���캯��===============================================================================================
//�ֶ�����·�������ơ���չ��������--------------------------------------------------------------
//���ն������ļ�
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
    //�����ַ����ļ�
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
    //����·�������ļ�ϵͳ�л�ȡ--------------------------------------------------------------------
    file::file(const std::string& filePath) :
        path(filePath),
        DataValid(false),
        PathValid(false)
    {
        // ʹ�� std::filesystem::path �������ļ�·��
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
    //===============================================================================================���캯��
    //Ԫ���ݻ�ȡ=============================================================================================
    // ��ȡ�ļ�������޸�ʱ�䲢��ʽ��Ϊ YYYY-MM-DD hh:mm:ss
    //std::string file::getTime() const {
    //    // ת���ļ�ʱ��Ϊϵͳʱ��
    //    auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(lastModifiedTime);
    //    std::time_t cftime = std::chrono::system_clock::to_time_t(systemTime);

    //    // ��ʱ���ʽ��Ϊ YYYY-MM-DD hh:mm:ss
    //    std::stringstream ss;
    //    ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
    //    return ss.str();
    //}
    // ��ȡ�ļ��Ĵ�С
    size_t file::size() const {
        if (!DataValid) {
            // �ӳټ����ļ�����
            auto mutableThis = const_cast<file*>(this);
            mutableThis->getData();
        }
        return data->size();
    }
    // ��ȡ�������ļ���������չ����
    std::string file::fullName() const {
        return std::string(name) + (ext.empty() ? "" : "." + std::string(ext));
    }
    std::string file::getName() const {
        return name;
    }
    std::string file::getExt() const {
        return ext;
    }
    // ��֤·���Ƿ���Ч
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
    // �ж��ļ������Ƿ���Ч
    bool file::isDataValid() const {
        return DataValid;
    }
    // �ж��ļ�·���Ƿ���Ч
    bool file::isPathValid() const {
        return PathValid;
    }
    std::string file::getMd5(){
        // ����Ѿ������ MD5����ֱ�ӷ��ػ���Ľ��
        if (!md5.empty()) {
            return md5;
        }
        try {
            MD5 md5Calculator; // ���� MD5 ���ʵ��
            // �ж��ļ������Ƿ��Ѿ����أ����δ�������������
            if (!DataValid) {
                getData();
            }
            // ��ȡ�ļ����ݲ����� MD5
            md5Calculator.update(data->data(), data->size()); // ʹ�� MD5 �����ÿһ�е�����
            md5Calculator.finalize();
            md5 = md5Calculator.hexdigest(); // ��ȡʮ������ժҪ
            std::transform(md5.begin(), md5.end(), md5.begin(), ::toupper); // �� MD5 ���תΪ��д
            return md5;
        }
        catch (...) {
            return "";
        }
    }
    //=============================================================================================Ԫ���ݻ�ȡ
    //���ݻ�ȡ===============================================================================================
    // �ӿڣ���ȡ����������-------------------------------------------------------------
    const std::vector<char> file::getData(){
        if (!DataValid) {
            data = std::make_shared<std::vector<char>>(readFileData());
        }
        return *data;
    }
    // �ӿڣ���ȡ�ı�����---------------------------------------------------------------
    const std::string file::getText() {
        if (!DataValid) {
            getData();  // �ӳټ�������
        }
        // ִ���в�ֺͻ��з�����
        return assembleText(*data);
    }
    // �����ļ���ʵ��
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
    //===============================================================================================���ݻ�ȡ
    // �������������ļ����ݰ��зָ�
    const std::vector<std::vector<char>> file::splitLines(const std::vector<char>& fileContent)const {
        std::vector<std::vector<char>> lines;
        // ����������ԼΪ1024���п��ԼΪ256�����Ը���ʵ���������
        size_t estimatedLineCount = 1024;
        size_t estimatedLineLength = 512;
        lines.reserve(estimatedLineCount);  // Ԥ�����������Ŀռ�
        std::vector<size_t> lineEndPositions;
        // ��¼���л��з���λ��
        for (size_t i = 0; i < fileContent.size(); ++i) {
            if (fileContent[i] == '\n') {
                lineEndPositions.push_back(i);
            }
        }
        // ����ÿһ�еĿռ䲢����ָ�
        size_t start = 0;
        for (size_t end : lineEndPositions) {
            // ����ÿ�еĳ��Ȳ�Ԥ����ռ�
            size_t lineLength = end - start + 1;
            std::vector<char> line;
            line.reserve(lineLength);  // Ϊÿ�з���ռ�
            line.insert(line.end(), fileContent.begin() + start, fileContent.begin() + end + 1);
            lines.push_back(std::move(line));
            start = end + 1;
        }
        // ����û���Ի��з���β�����һ��
        if (start < fileContent.size()) {
            std::vector<char> line;
            line.reserve(fileContent.size() - start);  // ���һ��
            line.insert(line.end(), fileContent.begin() + start, fileContent.end());
            lines.push_back(std::move(line));
        }
        return lines;
    }

    // ���������������зָ������������װΪ����������
    const std::vector<char> file::assembleBinary(const std::vector<std::vector<char>>& lines) const {
        std::vector<char> binaryData;
        binaryData.reserve(size());
        for (const auto& line : lines) {
            binaryData.insert(binaryData.end(), line.begin(), line.end());
        }
        return binaryData;
    }
    // ���������������зָ������������װΪ�ı����ݣ������з�
    const std::string file::assembleText(const std::vector<char>& data) const {
        std::string text;
        size_t i = 0;
        while (i < data.size()) {
            if (data[i] == '\n') {
                if (i > 0 && data[i - 1] == '\r') {
                    text.pop_back();  // ȥ�� '\r'������ '\n'
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