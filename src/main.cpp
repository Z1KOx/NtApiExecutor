#include "nt_api_executor.hpp"

typedef NTSTATUS( __stdcall* NtCreateFile_t )(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);

typedef VOID( __stdcall* RtlInitUnicodeString_t )( PUNICODE_STRING, PCWSTR );

int main()
{
    try
    {
        NtApiExecutor ntdll( skCrypt( L"ntdll.dll" ), skCrypt( "NtCreateFile" ) );
        const auto NtCreateFile = ntdll.resolveFunction<NtCreateFile_t>();
        if ( !NtCreateFile ) { throw std::runtime_error( "NtCreateFile not resolved." ); }

        NtApiExecutor rtl( skCrypt( L"ntdll.dll" ), skCrypt( "RtlInitUnicodeString" ) );
        const auto RtlInitUnicodeString = rtl.resolveFunction<RtlInitUnicodeString_t>();
        if ( !RtlInitUnicodeString ) { throw std::runtime_error( "RtlInitUnicodeString not resolved." ); }

        UNICODE_STRING uniStr = { 0 };
        OBJECT_ATTRIBUTES objAttr = { 0 };
        IO_STATUS_BLOCK ioStatus = { 0 };
        HANDLE hFile = nullptr;

        RtlInitUnicodeString( &uniStr, skCrypt( L"\\??\\C:\\test.txt" ) );
        InitializeObjectAttributes( &objAttr, &uniStr, OBJ_CASE_INSENSITIVE, nullptr, nullptr );

        ntdll.call(
            NtCreateFile,
            &hFile,
            FILE_GENERIC_WRITE,
            &objAttr,
            &ioStatus,
            nullptr,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OVERWRITE_IF,
            FILE_SYNCHRONOUS_IO_NONALERT,
            nullptr,
            0
        );

        if ( hFile ) { CloseHandle( hFile ); }
    }
    catch ( const std::exception& e ) {
        std::cout << e.what() << '\n';
    }

    return 0;
}