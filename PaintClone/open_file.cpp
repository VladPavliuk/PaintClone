#include "open_file.h"

bool ShowOpenFileDialog(WindowData* windowData, WideString* filePath)
{
    bool result = false;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            /* Set file types */
            COMDLG_FILTERSPEC fileTypes[] =
            {
                { L"All Files", L"*"},
                { L"All supported images (*.png | *.jpg | *.jpeg | *.psd)", L"*.png;*.jpg;*.jpeg;*.psd" },
            };
            hr = pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);

            // Show the Open dialog box.
            hr = pFileOpen->Show(windowData->parentHwnd);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        if (filePath->chars != 0) filePath->freeMemory();
                        *filePath = WideString(pszFilePath);
                        result = true;
                        //MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }

    return result;
}
