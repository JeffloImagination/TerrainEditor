#ifndef UTILS_H
#define UTILS_H


#include <windows.h>
#include <shlobj.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <imm.h>
#include <filesystem>
#include <shlwapi.h> // ����PathRemoveFileSpec��PathCombine

#pragma comment(lib, "Shlwapi.lib") // ����Shlwapi��

#define PROJECT_DIR_NAME "TerrainEditor"

// ���뷨ǿ����ΪӢ��
void SetEnglishInputLanguage();
// ��ȡ����ɫ�����ļ�
std::string ReadFile(const std::string& filePath);

// ��Windows�Ի���ѡ��һ���ļ���
std::wstring BrowseForFolder(HWND hwndOwner, const std::wstring& defaultPath = L"");
int CALLBACK BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData); // ���öԻ���Ĭ��·��

// ��Windows�Ի���ѡ��һ���ļ�
std::wstring BrowseForFile(HWND hwndOwner, const std::wstring& defaultFilePath);


// ��Ŀ���·��ת����·��
std::wstring GetAbsolutePathFromRelative(const std::wstring& relativePath);







#endif // UTILS_H