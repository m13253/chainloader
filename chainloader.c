/*
  MIT License

  Copyright (c) 2022 Star Brilliant

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>
#include <commctrl.h>

static PWSTR get_next_cmdline(void)
{
    size_t argi = 0;
    size_t num_backslashes = 0;
    bool is_quoted = false;

    LPWSTR cmdline = GetCommandLineW();
    for (size_t i = 0; cmdline && argi < 2; i++) {
        switch (cmdline[i]) {
        case L'\0':
            return NULL;
        case L'\t':
        case L' ':
            if (!is_quoted && (i == 0 || (cmdline[i - 1] != L'\t' && cmdline[i - 1] != L' '))) {
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
    return NULL;
}

static void exit_with_error_message(DWORD error_code)
{
    LPWSTR buffer = NULL;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&buffer,
            0,
            NULL)
        != 0) {
        MessageBoxExW(NULL, buffer, NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    };
    ExitProcess(error_code);
}

void WINAPI WinMainCRTStartup(void)
{
    InitCommonControls();

    PWSTR next_cmdline = get_next_cmdline();
    if (!next_cmdline) {
        MessageBoxExW(NULL, L"Next executable not specified.", NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
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
