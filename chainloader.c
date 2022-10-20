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
#include <stdnoreturn.h>

#define DECLSPEC_NORETURN noreturn
#include <windows.h>
/* Must be below windows.h */
#include <commctrl.h>

noreturn void WINAPI WinMainCRTStartup(void)
{
    DWORD exit_code;

    PROCESS_INFORMATION process_info;
    {
        LPWSTR cmdline = GetCommandLineW();
        // https://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?view=msvc-170
        if (!cmdline) {
            goto invalid_cmdline;
        }

    state_start : {
        switch (cmdline[0]) {
        case L'\0':
            goto invalid_cmdline;
        case L'\t':
        case L' ':
            cmdline++;
            goto state_space;
        case L'"':
            cmdline++;
            goto state_quote;
        default:
            cmdline++;
            goto state_start;
        }
    }

    state_quote : {
        switch (cmdline[0]) {
        case L'\0':
            goto invalid_cmdline;
        case L'"':
            cmdline++;
            goto state_start;
        default:
            cmdline++;
            goto state_quote;
        }
    }

    state_space : {
        switch (cmdline[0]) {
        case L'\0':
            goto invalid_cmdline;
        case L'\t':
        case L' ':
            cmdline++;
            goto state_space;
        default:
            goto start_child_process;
        }
    }

    start_child_process : {
        STARTUPINFOW startup_info;
        GetStartupInfoW(&startup_info);

        if (CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info) == FALSE) {
            exit_code = GetLastError();
            goto show_error_message;
        }
    }
    }

    CloseHandle(process_info.hThread);

    if (WaitForSingleObject(process_info.hProcess, INFINITE) != WAIT_OBJECT_0) {
        exit_code = GetLastError();
        goto show_error_message;
    }

    if (GetExitCodeProcess(process_info.hProcess, &exit_code) == FALSE) {
        exit_code = GetLastError();
        goto show_error_message;
    }

    CloseHandle(process_info.hProcess);
    goto exit;

invalid_cmdline : {
    InitCommonControls();
    MessageBoxExW(NULL, L"Next executable not specified.", NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    exit_code = ERROR_BAD_ARGUMENTS;
    goto exit;
}

show_error_message : {
    LPWSTR buffer = NULL;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            exit_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&buffer,
            0,
            NULL)
        != 0) {
        InitCommonControls();
        MessageBoxExW(NULL, buffer, NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    };
}

exit : {
    ExitProcess(exit_code);
}
}
