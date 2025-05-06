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
    void call( Func function, Args... args ) const noexcept;

    template< typename T >
    [[nodiscard]] T resolveFunction() noexcept;

private:
    [[nodiscard]] PVOID getModuleBaseAddr() noexcept;
    [[nodiscard]] DWORD getExportRva() noexcept;

private:
    PVOID m_modBase = nullptr;
    const wchar_t* m_modName = nullptr;
    const char* m_funcName = nullptr;
};

template< typename Func, typename ...Args >
inline void NtApiExecutor::call( Func function, Args ...args ) const noexcept
{
    NTSTATUS status = function( args... );
    if ( NT_SUCCESS( status ) ) {
        std::wcout << skCrypt( L"[*] Operation succeeded\n" ).decrypt();
    }
    else {
        std::wcout << skCrypt( L"[-] Failed (0x" ).decrypt()
                   << std::hex << status << L")\n";
    }
}

template<typename T>
inline T NtApiExecutor::resolveFunction() noexcept
{
    const auto rva = getExportRva();
    if (rva) {
        std::wcout << skCrypt( L"[+] Resolved " ).decrypt()
                   << m_funcName << skCrypt( L" : 0x" ).decrypt()
                   << std::hex << rva << skCrypt( L"\n\n" ).decrypt();
        return reinterpret_cast<T>( reinterpret_cast<BYTE*>( m_modBase ) + rva );
    }
    return nullptr;
}