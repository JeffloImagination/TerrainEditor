#include "Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <include/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <include/stb_image.h>


void SetEnglishInputLanguage()
{
    // 获取当前线程的输入上下文
    HIMC hIMC = ImmGetContext(GetForegroundWindow());
    if (hIMC)
    {
        // 获取当前输入语言
        HKL currentLayout = GetKeyboardLayout(0);

        // 定义英文输入法的布局
        HKL englishLayout = LoadKeyboardLayout(L"00000409", KLF_ACTIVATE); // 00000409 是英文（美国）的布局

        // 切换到英文输入法
        if (currentLayout != englishLayout)
        {
            ActivateKeyboardLayout(englishLayout, KLF_ACTIVATE);
        }

        // 释放输入上下文
        ImmReleaseContext(GetForegroundWindow(), hIMC);
    }
}

std::string ReadFile(const std::string& filePath)
{
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open())
    {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();

    return buffer.str();
}


// 回调函数用于设置对话框默认路径
int CALLBACK BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED && lpData != 0)
    {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

std::wstring BrowseForFolder(HWND hwndOwner, const std::wstring& defaultPath)
{
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hwndOwner;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = new TCHAR[MAX_PATH]; // 分配空间以存储显示名称
    bi.lpszTitle = L"请选择一个文件夹";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpfn = BrowseForFolderCallback; // 设置回调函数
    bi.lParam = (LPARAM)defaultPath.c_str(); // 将默认路径作为参数传递给回调函数
    bi.iImage = -1;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    std::wstring folderPath;

    if (pidl != NULL)
    {
        // 用户选择了文件夹。
        TCHAR path[MAX_PATH] = { 0 };
        if (SHGetPathFromIDList(pidl, path))
        {
            folderPath = path;
        }
        CoTaskMemFree(pidl); // 释放PIDL。
    }

    delete[] bi.pszDisplayName; // 释放分配的空间

    // 如果没有选择文件夹，则返回空字符串。
    return folderPath.empty() ? L"" : folderPath;
}


std::wstring BrowseForFile(HWND hwndOwner, const std::wstring& defaultFilePath)
{
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    // 设置默认文件路径
    if (!defaultFilePath.empty()) {
        wcsncpy_s(szFile, defaultFilePath.c_str(), MAX_PATH - 1);
        szFile[MAX_PATH - 1] = L'\0'; // 确保字符串以 null 结尾
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;

    // 如果提供了默认文件路径，则设置初始目录
    if (!defaultFilePath.empty()) {
        std::filesystem::path defPath(defaultFilePath);
        ofn.lpstrInitialDir = defPath.parent_path().wstring().c_str();
    }

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // 打开文件选择对话框
    if (GetOpenFileName(&ofn))
    {
        return ofn.lpstrFile;
    }

    return L""; // 返回空串表示用户取消了选择
}

std::wstring GetAbsolutePathFromRelative(const std::wstring& relativePath)
{
    // 获取可执行文件的路径
    wchar_t buffer[MAX_PATH];
    if (GetModuleFileName(NULL, buffer, MAX_PATH) == 0)
    {
        // 处理错误：可以通过抛出异常或返回空字符串
        return L"";
    }

    // 创建路径对象，并移除3个层级以到达项目根目录
    std::filesystem::path projectRoot(buffer);
    projectRoot = projectRoot.parent_path().parent_path().parent_path();

    // 构建默认路径：项目根目录 + PROJECT_DIR_NAME + 相对路径
    std::filesystem::path absolutePath = projectRoot / PROJECT_DIR_NAME / std::filesystem::path(relativePath);

    // 将结果转换为宽字符字符串并返回
    return absolutePath.wstring();
}






