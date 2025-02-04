#pragma once
#include <string>
#include <vector>
#include <memory>

namespace DC {
    class file {
    public:
        //���캯��==========================================================================
        file() : DataValid(false), PathValid(false) {}
        file(
            std::string ext,                // ��չ��
            std::vector<char> content,      // ����
            std::string path = "",          // ·������ʡ��
            std::string name = ""           // �ļ�������ʡ��
        );
        file(
            std::string ext,                // ��չ��
            std::string content,            // ����
            std::string path = "",          // ·������ʡ��
            std::string name = ""           // �ļ�������ʡ��
        );
        file(
            const std::string& filePath     // ·��
        );
        //==========================================================================���캯��
        //�ӿڷ���==========================================================================
        size_t size() const;                // ��ȡ�ļ���С
        std::string fullName() const;       // ��ȡ�������ļ���������չ����
        std::string getName() const;        // ��ȡ�ļ���
        std::string getExt() const;         // ��ȡ��չ��
        // std::string getTime() const;        // ��ȡ�޸�ʱ��
        const std::vector<char> getData();  // ��ȡ�������ļ�����
        const std::string getText();        // ��ȡ�ı��ļ�����
        std::string getMd5();

        bool isDataValid() const;           // �ж��ļ������Ƿ���Ч
        bool isPathValid() const;           // �ж��ļ�·���Ƿ���Ч
        //==========================================================================�ӿڷ���
    private:
        std::string path;  // �ļ�·��
        std::string name;  // �ļ���
        std::string ext;   // ��չ��
        std::string md5;   // MD5��
        std::shared_ptr<std::vector<char>> data;  // �ļ�����
        // std::filesystem::file_time_type lastModifiedTime; // ʱ���

        bool DataValid; // �ļ������Ƿ���Ч
        bool PathValid; // �ļ�·���Ƿ���Ч

        void validatePath();
        const std::vector<std::vector<char>> splitLines(const std::vector<char>& fileContent)const;
        const std::vector<char> assembleBinary(const std::vector<std::vector<char>>& lines)const;
        const std::string assembleText(const std::vector<char>& data)const;
        const std::vector<char> readFileData();
    };
}