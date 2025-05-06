#pragma once

#include <windows.h>
#include <iostream>
#include "typedefs.h"
#include "skCrypt.h"

class NtApiExecutor 
{
public:
    NtApiExecutor( const wchar_t* modName, const char* funcName );
    ~NtApiExecutor() = default;

    NtApiExecutor( const NtApiExecutor& ) = delete;
    NtApiExecutor( NtApiExecutor&& ) = delete;
    NtApiExecutor& operator=( const NtApiExecutor& ) = delete;
    NtApiExecutor& operator=( NtApiExecutor&& ) = delete;

public:
    template< typename Func, typename... Args >
    void callNtFunction( Func function, Args... args ) const noexcept 
    {
        NTSTATUS status = function( args... );
        if ( NT_SUCCESS( status ) ) {
            std::wcout << skCrypt( L"[+] Operation succeeded" ).decrypt() << skCrypt( L"\n" ).decrypt();
        }
        else {
            std::wcout << skCrypt( L"[-] Failed (0x" ).decrypt() << std::hex << status << skCrypt( L"\n" ).decrypt();
        }
    }

    template< typename T >
    T resolveFunction() noexcept
    {
        DWORD rva = getExportRva();
        return rva ? reinterpret_cast<T>( reinterpret_cast<BYTE*>( m_modBase ) + rva ) 
                   : nullptr;
    }

private:
    PVOID getModuleBaseAddr() noexcept;
    DWORD getExportRva() noexcept;

private:
    PVOID m_modBase = nullptr;
    const wchar_t* m_modName = nullptr;
    const char* m_funcName = nullptr;
};