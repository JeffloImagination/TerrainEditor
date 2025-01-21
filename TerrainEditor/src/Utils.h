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
#include <shlwapi.h> // 用于PathRemoveFileSpec和PathCombine

#pragma comment(lib, "Shlwapi.lib") // 链接Shlwapi库

#define PROJECT_DIR_NAME "TerrainEditor"

// 输入法强制设为英文
void SetEnglishInputLanguage();
// 读取（着色器）文件
std::string ReadFile(const std::string& filePath);

// 打开Windows对话框，选择一个文件夹
std::wstring BrowseForFolder(HWND hwndOwner, const std::wstring& defaultPath = L"");
int CALLBACK BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData); // 设置对话框默认路径

// 打开Windows对话框，选择一个文件
std::wstring BrowseForFile(HWND hwndOwner, const std::wstring& defaultFilePath);


// 项目相对路径转绝对路径
std::wstring GetAbsolutePathFromRelative(const std::wstring& relativePath);







#endif // UTILS_H