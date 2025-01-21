#include "Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <include/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <include/stb_image.h>


void SetEnglishInputLanguage()
{
    // ��ȡ��ǰ�̵߳�����������
    HIMC hIMC = ImmGetContext(GetForegroundWindow());
    if (hIMC)
    {
        // ��ȡ��ǰ��������
        HKL currentLayout = GetKeyboardLayout(0);

        // ����Ӣ�����뷨�Ĳ���
        HKL englishLayout = LoadKeyboardLayout(L"00000409", KLF_ACTIVATE); // 00000409 ��Ӣ�ģ��������Ĳ���

        // �л���Ӣ�����뷨
        if (currentLayout != englishLayout)
        {
            ActivateKeyboardLayout(englishLayout, KLF_ACTIVATE);
        }

        // �ͷ�����������
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


// �ص������������öԻ���Ĭ��·��
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
    bi.pszDisplayName = new TCHAR[MAX_PATH]; // ����ռ��Դ洢��ʾ����
    bi.lpszTitle = L"��ѡ��һ���ļ���";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpfn = BrowseForFolderCallback; // ���ûص�����
    bi.lParam = (LPARAM)defaultPath.c_str(); // ��Ĭ��·����Ϊ�������ݸ��ص�����
    bi.iImage = -1;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    std::wstring folderPath;

    if (pidl != NULL)
    {
        // �û�ѡ�����ļ��С�
        TCHAR path[MAX_PATH] = { 0 };
        if (SHGetPathFromIDList(pidl, path))
        {
            folderPath = path;
        }
        CoTaskMemFree(pidl); // �ͷ�PIDL��
    }

    delete[] bi.pszDisplayName; // �ͷŷ���Ŀռ�

    // ���û��ѡ���ļ��У��򷵻ؿ��ַ�����
    return folderPath.empty() ? L"" : folderPath;
}


std::wstring BrowseForFile(HWND hwndOwner, const std::wstring& defaultFilePath)
{
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    // ����Ĭ���ļ�·��
    if (!defaultFilePath.empty()) {
        wcsncpy_s(szFile, defaultFilePath.c_str(), MAX_PATH - 1);
        szFile[MAX_PATH - 1] = L'\0'; // ȷ���ַ����� null ��β
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;

    // ����ṩ��Ĭ���ļ�·���������ó�ʼĿ¼
    if (!defaultFilePath.empty()) {
        std::filesystem::path defPath(defaultFilePath);
        ofn.lpstrInitialDir = defPath.parent_path().wstring().c_str();
    }

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // ���ļ�ѡ��Ի���
    if (GetOpenFileName(&ofn))
    {
        return ofn.lpstrFile;
    }

    return L""; // ���ؿմ���ʾ�û�ȡ����ѡ��
}

std::wstring GetAbsolutePathFromRelative(const std::wstring& relativePath)
{
    // ��ȡ��ִ���ļ���·��
    wchar_t buffer[MAX_PATH];
    if (GetModuleFileName(NULL, buffer, MAX_PATH) == 0)
    {
        // ������󣺿���ͨ���׳��쳣�򷵻ؿ��ַ���
        return L"";
    }

    // ����·�����󣬲��Ƴ�3���㼶�Ե�����Ŀ��Ŀ¼
    std::filesystem::path projectRoot(buffer);
    projectRoot = projectRoot.parent_path().parent_path().parent_path();

    // ����Ĭ��·������Ŀ��Ŀ¼ + PROJECT_DIR_NAME + ���·��
    std::filesystem::path absolutePath = projectRoot / PROJECT_DIR_NAME / std::filesystem::path(relativePath);

    // �����ת��Ϊ���ַ��ַ���������
    return absolutePath.wstring();
}






