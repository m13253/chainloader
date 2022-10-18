#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>

static PWSTR get_next_cmdline(void)
{
    size_t argi = 0;
    size_t num_backslashes = 0;
    bool is_quoted = false;

    LPWSTR cmdline = GetCommandLineW();
    for (size_t i = 0; cmdline && argi < 2; i++) {
        WCHAR c = cmdline[i];
        switch (c) {
        case L'\0':
            return NULL;
        case L'\t':
        case L' ':
            if (!is_quoted && (i == 0 || cmdline[i - 1] != L'\t' && cmdline[i - 1] != L' ')) {
                argi++;
            }
            num_backslashes = 0;
            break;
        case L'\\':
            if (argi == 1) {
                return &cmdline[i];
            }
            num_backslashes++;
            break;
        case L'"':
            if (argi == 1) {
                return &cmdline[i];
            }
            if (num_backslashes % 2 == 0) {
                is_quoted = !is_quoted;
            }
            num_backslashes = 0;
            break;
        default:
            if (argi == 1) {
                return &cmdline[i];
            }
            num_backslashes = 0;
        }
    }
}

static void exit_with_error_message(DWORD error_code)
{
    LPWSTR buffer = NULL;
    if (FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&buffer,
            0,
            NULL)
        == FALSE) {
        ExitProcess(error_code);
    };
    MessageBoxExW(NULL, buffer, L"Error", MB_OK | MB_ICONERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    ExitProcess(error_code);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    PWSTR next_cmdline = get_next_cmdline();
    if (!next_cmdline) {
        MessageBoxExW(NULL, L"Next executable not specified.", L"Error", MB_OK | MB_ICONERROR, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
        ExitProcess(ERROR_BAD_ARGUMENTS);
    }

    STARTUPINFOW startup_info;
    GetStartupInfoW(&startup_info);

    PROCESS_INFORMATION process_info;
    if (CreateProcessW(NULL, next_cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info) == FALSE) {
        DWORD error_code = GetLastError();
        exit_with_error_message(error_code);
    }
    CloseHandle(process_info.hThread);

    if (WaitForSingleObject(process_info.hProcess, INFINITE) != WAIT_OBJECT_0) {
        DWORD error_code = GetLastError();
        CloseHandle(process_info.hProcess);
        exit_with_error_message(error_code);
    }
    DWORD exit_code;
    if (GetExitCodeProcess(process_info.hProcess, &exit_code) == FALSE) {
        DWORD error_code = GetLastError();
        CloseHandle(process_info.hProcess);
        exit_with_error_message(error_code);
    }
    CloseHandle(process_info.hProcess);

    ExitProcess(exit_code);
}
