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
        LPWSTR cmdline;
        {
            size_t argi = 0;
            bool is_quoted = false;
            bool is_num_backslashes_odd = false;

            cmdline = GetCommandLineW();
            if (!cmdline) {
                goto invalid_cmdline;
            }

            for (size_t i = 0;; i++) {
                switch (cmdline[i]) {
                case L'\0':
                    goto invalid_cmdline;
                case L'\t':
                case L' ':
                    if (!is_quoted && (i == 0 || (cmdline[i - 1] != L'\t' && cmdline[i - 1] != L' '))) {
                        argi++;
                    }
                    is_num_backslashes_odd = false;
                    break;
                case L'"':
                    if (argi > 0) {
                        cmdline = &cmdline[i];
                        goto start_child_process;
                    }
                    if (!is_num_backslashes_odd) {
                        is_quoted = !is_quoted;
                    }
                    is_num_backslashes_odd = false;
                    break;
                case L'\\':
                    if (argi > 0) {
                        cmdline = &cmdline[i];
                        goto start_child_process;
                    }
                    is_num_backslashes_odd = !is_num_backslashes_odd;
                    break;
                default:
                    if (argi > 0) {
                        cmdline = &cmdline[i];
                        goto start_child_process;
                    }
                    is_num_backslashes_odd = false;
                }
            }
            goto invalid_cmdline;
        }

    start_child_process : {
        MessageBoxExW(NULL, cmdline, NULL, MB_OK | MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));

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
