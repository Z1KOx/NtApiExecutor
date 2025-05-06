#pragma once

#include <windows.h>
#include <iostream>
#include "misc/typedefs.h"
#include "misc/skCrypt.h"

class NtApiExecutor 
{
public:
    // Constructor that initializes the module and function names
    explicit NtApiExecutor( const wchar_t* modName, const char* funcName );
    ~NtApiExecutor() = default;

    // Deleting copy and move constructors to prevent copying of the executor
    NtApiExecutor( const NtApiExecutor& ) = delete;
    NtApiExecutor( NtApiExecutor&& ) = delete;
    NtApiExecutor& operator=( const NtApiExecutor& ) = delete;
    NtApiExecutor& operator=( NtApiExecutor&& ) = delete;

public:
    // Template method to call a function with specified arguments
    template< typename Func, typename... Args >
    void call( Func function, Args... args ) const noexcept;

    // Template method to resolve the function pointer from the module
    template< typename T >
    [[nodiscard]] T resolveFunction() noexcept;

private:
    // Private methods to retrieve the module's base address and function's RVA
    [[nodiscard]] PVOID getModuleBaseAddr() noexcept;
    [[nodiscard]] DWORD getExportRva() noexcept;

private:
    PVOID m_modBase = nullptr;
    const wchar_t* m_modName = nullptr;
    const char* m_funcName = nullptr;
};

// Calls the NT function and outputs success or failure status
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

// Resolves function pointer from RVA and returns it
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