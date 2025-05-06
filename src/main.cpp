#include "nt_api_executor.hpp"

typedef NTSTATUS( __stdcall* NtCreateFile_t )(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);

typedef VOID( __stdcall* RtlInitUnicodeString_t )( PUNICODE_STRING, PCWSTR );

int main()
{
    NtApiExecutor ntdll( skCrypt( L"ntdll.dll" ), skCrypt( "NtCreateFile" ) );
    auto NtCreateFile = ntdll.resolveFunction<NtCreateFile_t>();

    NtApiExecutor rtl(skCrypt( L"ntdll.dll" ), skCrypt( "RtlInitUnicodeString" ) );
    auto RtlInitUnicodeString = rtl.resolveFunction<RtlInitUnicodeString_t>();

    if (!NtCreateFile || !RtlInitUnicodeString) {
        std::wcerr << skCrypt( L"[-] Failed to resolve functions\n" ).decrypt();
        return 1;
    }

    UNICODE_STRING uniStr;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK ioStatus;
    HANDLE hFile = nullptr;

    RtlInitUnicodeString( &uniStr, skCrypt( L"\\??\\C:\\test.txt" ) );
    InitializeObjectAttributes(&objAttr, &uniStr, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    ntdll.callNtFunction(
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

    if (hFile) { CloseHandle(hFile); }
    return 0;
}