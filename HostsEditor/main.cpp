#include <windows.h>

#include <string>
#include <vector>

#pragma comment(lib, "user32.lib")

namespace {

constexpr const wchar_t* kGuiExeName = L"HostsEditor-GUI.exe";

std::wstring GetLocalAppDataDir()
{
    wchar_t buf[MAX_PATH] = {};
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        return L"";
    }
    return std::wstring(buf);
}

bool WriteFileBytes(const std::wstring& path, const void* data, DWORD size)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD written = 0;
    BOOL ok = WriteFile(file, data, size, &written, nullptr);
    CloseHandle(file);
    return ok && written == size;
}

struct ExtractContext
{
    std::wstring targetDir;
    int failures;
};

BOOL CALLBACK EnumerateResource(HMODULE module, LPCWSTR type, LPWSTR name, LONG_PTR param)
{
    if (IS_INTRESOURCE(name))
    {
        return TRUE;
    }

    auto* context = reinterpret_cast<ExtractContext*>(param);

    HRSRC res = FindResourceW(module, name, type);
    if (res == nullptr)
    {
        ++context->failures;
        return TRUE;
    }

    HGLOBAL data = LoadResource(module, res);
    if (data == nullptr)
    {
        ++context->failures;
        return TRUE;
    }

    void* ptr = LockResource(data);
    DWORD size = SizeofResource(module, res);
    if (ptr == nullptr || size == 0)
    {
        ++context->failures;
        return TRUE;
    }

    std::wstring fileName(name);
    if (fileName.size() >= 2 && fileName.front() == L'"' && fileName.back() == L'"')
    {
        fileName = fileName.substr(1, fileName.size() - 2);
    }
    if (fileName.empty())
    {
        ++context->failures;
        return TRUE;
    }

    std::wstring target = context->targetDir + L"\\" + fileName;
    if (!WriteFileBytes(target, ptr, size))
    {
        ++context->failures;
    }

    return TRUE;
}

void SetAttributesRecursive(const std::wstring& path)
{
    WIN32_FIND_DATAW fd;
    std::wstring pattern = path + L"\\*";
    HANDLE find = FindFirstFileW(pattern.c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
        {
            continue;
        }

        std::wstring full = path + L"\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            SetAttributesRecursive(full);
        }
        else
        {
            DWORD attrs = GetFileAttributesW(full.c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY))
            {
                SetFileAttributesW(full.c_str(), attrs & ~FILE_ATTRIBUTE_READONLY);
            }
        }
    } while (FindNextFileW(find, &fd));

    FindClose(find);
}

bool RemoveDirectoryRecursive(const std::wstring& path)
{
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        return true;
    }

    SetAttributesRecursive(path);

    WIN32_FIND_DATAW fd;
    std::wstring pattern = path + L"\\*";
    HANDLE find = FindFirstFileW(pattern.c_str(), &fd);
    if (find != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            {
                continue;
            }

            std::wstring full = path + L"\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                RemoveDirectoryRecursive(full);
            }
            else
            {
                DeleteFileW(full.c_str());
            }
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }

    return RemoveDirectoryW(path.c_str()) != FALSE;
}

bool RunGuiAndWait(const std::wstring& exePath, const std::wstring& workDir, DWORD& exitCode)
{
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(exePath.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr,
                        workDir.c_str(), &si, &pi))
    {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    exitCode = code;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

}  // namespace

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\HostsEditorLauncherMutex");
    if (mutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 1;
    }

    std::wstring baseDir = GetLocalAppDataDir();
    if (baseDir.empty())
    {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
        return 2;
    }

    std::wstring appDir = baseDir + L"\\HostsEditor";
    RemoveDirectoryRecursive(appDir);

    if (!CreateDirectoryW(appDir.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
        return 3;
    }

    ExtractContext context{ appDir, 0 };
    EnumResourceNamesW(nullptr, RT_RCDATA, EnumerateResource,
                       reinterpret_cast<LONG_PTR>(&context));

    if (context.failures > 0)
    {
        MessageBoxW(nullptr, L"资源释放失败，无法启动 HostsEditor-GUI。", L"HostsEditor",
                    MB_OK | MB_ICONERROR);
        RemoveDirectoryRecursive(appDir);
        ReleaseMutex(mutex);
        CloseHandle(mutex);
        return 4;
    }

    std::wstring guiPath = appDir + L"\\" + kGuiExeName;
    DWORD exitCode = 0;
    if (!RunGuiAndWait(guiPath, appDir, exitCode))
    {
        MessageBoxW(nullptr, L"启动 HostsEditor-GUI 失败。", L"HostsEditor",
                    MB_OK | MB_ICONERROR);
    }

    int cleanupTries = 0;
    while (!RemoveDirectoryRecursive(appDir) && cleanupTries < 50)
    {
        ++cleanupTries;
        Sleep(100);
    }
    RemoveDirectoryRecursive(appDir);

    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return static_cast<int>(exitCode);
}
