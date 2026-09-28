#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <string>

static std::wstring A2W(const char* s)
{
    if (!s || !*s)
        return L"";

    int n = MultiByteToWideChar(
        CP_ACP, 0, s, -1, nullptr, 0);

    if (n <= 0)
        return L"";

    std::wstring w(n, L'\0');

    if (MultiByteToWideChar(
        CP_ACP, 0, s, -1, &w[0], n) <= 0)
        return L"";

    w.resize(n - 1);
    return w;
}

static bool W2A(
    const wchar_t* ws,
    char* buffer,
    int bufferSize)
{
    if (!ws || !buffer || bufferSize <= 0)
        return false;

    int n = WideCharToMultiByte(
        CP_ACP, 0, ws, -1,
        buffer, bufferSize,
        nullptr, nullptr);

    if (n <= 0)
    {
        buffer[0] = '\0';
        return false;
    }

    buffer[bufferSize - 1] = '\0';
    return true;
}

static std::wstring CurrentFolder()
{
    DWORD n = GetCurrentDirectoryW(0, nullptr);

    if (n == 0)
        return L"";

    std::wstring folder(n, L'\0');

    DWORD copied = GetCurrentDirectoryW(
        n, &folder[0]);

    if (copied == 0 || copied >= n)
        return L"";

    folder.resize(copied);
    return folder;
}

static bool FolderExists(const std::wstring& folder)
{
    if (folder.empty())
        return false;

    DWORD attr = GetFileAttributesW(
        folder.c_str());

    return attr != INVALID_FILE_ATTRIBUTES &&
           (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static HRESULT SetInitialFolder(
    IFileDialog* dialog,
    const std::wstring& folder)
{
    if (!dialog || folder.empty())
        return E_INVALIDARG;

    IShellItem* item = nullptr;

    HRESULT hr = SHCreateItemFromParsingName(
        folder.c_str(),
        nullptr,
        IID_PPV_ARGS(&item));

    if (FAILED(hr) || !item)
        return hr;

    hr = dialog->SetFolder(item);

    if (SUCCEEDED(hr))
        dialog->SetDefaultFolder(item);

    item->Release();

    /*
       En FOS_PICKFOLDERS, SetFileName hace que el campo
       inferior del diálogo muestre la carpeta inicial.
       El resultado real siempre se obtiene de GetResult().
    */
    dialog->SetFileName(folder.c_str());

    return hr;
}

extern "C"
__declspec(dllexport)
int __cdecl SelectFolder(
    HWND hOwner,
    char* cFolder,
    int nBufferSize,
    const char* cInitialFolder,
    const char* cTitle)
{
    if (!cFolder || nBufferSize <= 0)
        return 0;

    cFolder[0] = '\0';

    HRESULT hrCo = CoInitializeEx(
        nullptr,
        COINIT_APARTMENTTHREADED);

    bool uninitialize =
        SUCCEEDED(hrCo);

    if (FAILED(hrCo) &&
        hrCo != RPC_E_CHANGED_MODE)
        return 0;

    IFileOpenDialog* dialog = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&dialog));

    if (FAILED(hr) || !dialog)
    {
        if (uninitialize)
            CoUninitialize();

        return 0;
    }

    DWORD options = 0;

    if (SUCCEEDED(dialog->GetOptions(&options)))
    {
        options |= FOS_PICKFOLDERS;
        options |= FOS_FORCEFILESYSTEM;
        options |= FOS_PATHMUSTEXIST;
        options |= FOS_NOCHANGEDIR;

        dialog->SetOptions(options);
    }

    std::wstring title = A2W(cTitle);

    if (title.empty())
        title = L"Seleccionar carpeta";

    dialog->SetTitle(title.c_str());

    std::wstring initial =
        A2W(cInitialFolder);

    /*
       Si no se recibe carpeta inicial, o la recibida no
       existe, se utiliza el directorio actual del proceso.
    */
    if (!FolderExists(initial))
        initial = CurrentFolder();

    if (FolderExists(initial))
        SetInitialFolder(dialog, initial);

    hr = dialog->Show(hOwner);

    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        dialog->Release();

        if (uninitialize)
            CoUninitialize();

        return 0;
    }

    if (FAILED(hr))
    {
        dialog->Release();

        if (uninitialize)
            CoUninitialize();

        return 0;
    }

    IShellItem* result = nullptr;

    hr = dialog->GetResult(&result);

    if (FAILED(hr) || !result)
    {
        dialog->Release();

        if (uninitialize)
            CoUninitialize();

        return 0;
    }

    PWSTR selected = nullptr;

    hr = result->GetDisplayName(
        SIGDN_FILESYSPATH,
        &selected);

    int ok = 0;

    if (SUCCEEDED(hr) && selected)
    {
        if (W2A(
            selected,
            cFolder,
            nBufferSize))
        {
            ok = 1;
        }
    }

    if (selected)
        CoTaskMemFree(selected);

    result->Release();
    dialog->Release();

    if (uninitialize)
        CoUninitialize();

    return ok;
}
