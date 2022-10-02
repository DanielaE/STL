// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Implements a win32 API wrapper for <print>

// This must be as small as possible, because its contents are
// injected into the msvcprt.lib and msvcprtd.lib import libraries.
// Do not include or define anything else here.
// In particular, basic_string must not be included here.

#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <xfilesystem_abi.h>

#include <Windows.h>

void __std_print_notranscode(FILE * _File, const char * _Text, _STD size_t _TextLen) {
    fwrite(_Text, 1, _TextLen, _File);
}

static constexpr int _SmallBuffer = 500;

void __std_print(FILE * _File, const char * _Text, _STD size_t _TextLen,
                 unsigned _Codepage) {
    DWORD _Dummy = 0;
    HANDLE _Handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_File)));
    if (GetConsoleMode(_Handle, &_Dummy)) {
        const int _Size = MultiByteToWideChar(_Codepage, 0,
                                              _Text, static_cast<int>(_TextLen),
                                              nullptr, 0);
        if (_Size > _SmallBuffer) {
            if (wchar_t * _Buffer = new wchar_t[_Size]) {
                MultiByteToWideChar(_Codepage, 0, _Text, static_cast<int>(_TextLen),
                                    _Buffer, _Size);
                const int _Ret = WriteConsoleW(_Handle, _Buffer, _Size, &_Dummy, nullptr);
                delete[] _Buffer;
                if (_Ret)
                    return;
            }
        } else if (_Size > 0) {
            wchar_t * _Buffer = reinterpret_cast<wchar_t *>(_alloca(_Size * 2));
            MultiByteToWideChar(_Codepage, 0, _Text, static_cast<int>(_TextLen),
                                _Buffer, _Size);
            if (WriteConsoleW(_Handle, _Buffer, _Size, &_Dummy, nullptr))
                return;
        } else {
            return;
        }
    }
    __std_print_notranscode(_File, _Text, _TextLen);
}

void __std_print(FILE * _File, const wchar_t * _Text, _STD size_t _TextLen) {
    DWORD _Dummy = 0;
    HANDLE _Handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_File)));
    if (GetConsoleMode(_Handle, &_Dummy) &&
        WriteConsoleW(_Handle, _Text, static_cast<DWORD>(_TextLen), &_Dummy, nullptr))
            return;
    fwrite(_Text, 2, _TextLen, _File);
}
