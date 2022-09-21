// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Implements a win32 API wrapper for <format>

// This must be as small as possible, because its contents are
// injected into the msvcprt.lib and msvcprtd.lib import libraries.
// Do not include or define anything else here.
// In particular, basic_string must not be included here.

#include <io.h>
#include <stdio.h>
#include <xfilesystem_abi.h>

#include <Windows.h>

void __std_print(FILE * _File, const char * _Text, _STD size_t _TSize) {
    DWORD _Dummy = 0;
    HANDLE _Handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_File)));
    if (GetConsoleMode(_Handle, &_Dummy)) {
        const int _Size = MultiByteToWideChar(_MSVC_EXECUTION_CHARACTER_SET, 0, _Text,
                                              static_cast<int>(_TSize), nullptr, 0);
        static constexpr int _SmallBuffer = 80;
        if (_Size > _SmallBuffer) {
            if (wchar_t * _Buffer = new wchar_t[_Size]) {
                MultiByteToWideChar(_MSVC_EXECUTION_CHARACTER_SET, 0, _Text,
                                    static_cast<int>(_TSize), _Buffer, _Size);
                const int _Ret = WriteConsoleW(_Handle, _Buffer, _Size, &_Dummy, nullptr);
                delete[] _Buffer;
                if (_Ret)
                    return;
            }
        } else if (_Size > 0) {
            wchar_t _Buffer[_SmallBuffer];
            MultiByteToWideChar(_MSVC_EXECUTION_CHARACTER_SET, 0, _Text,
                                static_cast<int>(_TSize), _Buffer, _Size);
            if (WriteConsoleW(_Handle, _Buffer, _Size, &_Dummy, nullptr))
                return;
        } else {
            return;
        }
    }
    fwrite(_Text, 1, _TSize, _File);
}

void __std_print(FILE * _File, const wchar_t * _Text, _STD size_t _TSize) {
    DWORD _Dummy = 0;
    HANDLE _Handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_File)));
    if (GetConsoleMode(_Handle, &_Dummy) &&
        WriteConsoleW(_Handle, _Text, static_cast<DWORD>(_TSize), &_Dummy, nullptr))
            return;
    fwrite(_Text, 2, _TSize, _File);
}
