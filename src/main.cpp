#include <iostream>
#include "typedefs.h"
#include "skCrypt.h"

using RtlInitUnicodeString_t = VOID( __stdcall* )( PUNICODE_STRING, PCWSTR );
using NtCreateFile_t = NTSTATUS( __stdcall* )(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG
);

PVOID getModuleBaseAddress(
    const wchar_t* moduleName
) noexcept
{
    PPEB peb;
    ZeroMemory( &peb, sizeof( peb ) );
#ifdef _WIN64
    peb = reinterpret_cast<PPEB>( __readgsqword( 0x60 ) );
#else
    __asm {
        mov eax, fs: [0x30]
        mov peb, eax
    };
#endif
    if ( !peb || !peb->Ldr ) { return nullptr; }

    auto* head = &peb->Ldr->InMemoryOrderModuleList;
    auto* curr = head->Flink;

    while ( curr != head )
    {
        auto* entry{ CONTAINING_RECORD( curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks ) };

        if ( entry->BaseDllName.Buffer &&
            _wcsicmp( entry->BaseDllName.Buffer, moduleName ) == 0 ) {
            return entry->DllBase;
        }

        curr = curr->Flink;
    }

    return nullptr;
}

[[nodiscard]] DWORD getFunctionRVA(
    PVOID modBase,
    const char* funcName
) noexcept
{
    auto* dos{ reinterpret_cast<IMAGE_DOS_HEADER*>( modBase ) };
    if ( dos->e_magic != IMAGE_DOS_SIGNATURE ) { return 0; }

    auto* nt{ reinterpret_cast<IMAGE_NT_HEADERS*>(
        reinterpret_cast<BYTE*>( modBase ) + dos->e_lfanew ) };
    if ( nt->Signature != IMAGE_NT_SIGNATURE ) { return 0; }

    auto* exp{ reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
        reinterpret_cast<BYTE*>( modBase ) +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress ) };

    auto* names{ reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( modBase ) + exp->AddressOfNames ) };
    auto* funcs{ reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( modBase ) + exp->AddressOfFunctions ) };
    auto* ordinals{ reinterpret_cast<WORD*>(
        reinterpret_cast<BYTE*>( modBase ) + exp->AddressOfNameOrdinals ) };

    for ( DWORD i{ 0 }; i < exp->NumberOfNames; ++i )
    {
        auto* name{ reinterpret_cast<const char*>(
            reinterpret_cast<BYTE*>( modBase ) + names[i] ) };

        if ( _stricmp( name, funcName ) == 0 ) {
            return funcs[ordinals[i]];
        }
    }

    return 0;
}

template<typename Fn>
Fn resolveFunction(
    PVOID moduleBase,
    const char* funcName
) noexcept
{
    DWORD rva{ getFunctionRVA( moduleBase, funcName ) };
    if ( !rva ) { return nullptr; }
    return reinterpret_cast<Fn>( reinterpret_cast<BYTE*>( moduleBase ) + rva );
}

void callFunc(
    NtCreateFile_t NtCreateFileFn,
    RtlInitUnicodeString_t RtlInitUnicodeStringFn
) noexcept
{
    UNICODE_STRING uniStr;
    ZeroMemory( &uniStr, sizeof( uniStr ) );
    OBJECT_ATTRIBUTES objAttr;
    ZeroMemory( &objAttr, sizeof( objAttr ) );
    IO_STATUS_BLOCK ioStatus;
    ZeroMemory( &ioStatus, sizeof( ioStatus ) );
    HANDLE hFile{ nullptr };

    RtlInitUnicodeStringFn( &uniStr, skCrypt( L"\\??\\C:\\Windows\\Temp\\test.txt" ) );

    InitializeObjectAttributes( &objAttr, &uniStr, OBJ_CASE_INSENSITIVE, nullptr, nullptr );

    NTSTATUS status{ NtCreateFileFn(
        &hFile,
        FILE_GENERIC_WRITE | FILE_GENERIC_READ,
        &objAttr,
        &ioStatus,
        nullptr,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OVERWRITE_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        nullptr,
        0
    ) };

    if ( NT_SUCCESS( status ) ) {
        std::cout << skCrypt( "[+] File created successfully\n" );
        CloseHandle( hFile );
    }
    else {
        std::cout << skCrypt( "[-] NtCreateFile failed: 0x" ) << std::hex << status << '\n';
    }
}

int main()
{
    const auto ntdllBase{ getModuleBaseAddress( skCrypt( L"ntdll.dll" ) ) };
    if ( !ntdllBase ) {
        std::cerr << skCrypt( "[-] Couldn't load ntdll.dll\n" );
        return -1;
    }

    const auto NtCreateFileFn{ resolveFunction<NtCreateFile_t>(
        ntdllBase,
        skCrypt( "NtCreateFile" )
    ) };
    const auto RtlInitUnicodeStringFn{ resolveFunction<RtlInitUnicodeString_t>(
        ntdllBase,
        skCrypt( "RtlInitUnicodeString" )
    ) };

    if ( !NtCreateFileFn || !RtlInitUnicodeStringFn ) {
        std::cerr << skCrypt( "[-] Could not resolve required functions\n" );
        return -1;
    }

    callFunc( NtCreateFileFn, RtlInitUnicodeStringFn );

    return 0;
}