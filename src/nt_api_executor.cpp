#include "nt_api_executor.hpp"

NtApiExecutor::NtApiExecutor( const wchar_t* modName, const char* funcName )
    : m_modName( modName ), m_funcName( funcName )
{
    m_modBase = getModuleBaseAddr();
    if ( !m_modBase ) {
        std::wcerr << skCrypt( L"[-] Failed to locate module: " ) << modName << skCrypt( L"\n" );
    }
}

[[nodiscard]] PVOID 
    NtApiExecutor::getModuleBaseAddr() noexcept
{
    PPEB peb;
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
        auto* entry = CONTAINING_RECORD( curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );

        if ( entry->BaseDllName.Buffer &&
            _wcsicmp( entry->BaseDllName.Buffer, m_modName ) == 0 ) {
            return entry->DllBase;
        }
        curr = curr->Flink;
    }
    return nullptr;
}

[[nodiscard]] DWORD 
    NtApiExecutor::getExportRva() noexcept 
{
    if ( !m_modBase ) { return 0; }

    auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>( m_modBase );
    auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<BYTE*>( m_modBase ) + dosHeader->e_lfanew );

    auto exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
        reinterpret_cast<BYTE*>( m_modBase ) +
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress );

    auto names = reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfNames );
    auto functions = reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfFunctions );
    auto ordinals = reinterpret_cast<WORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfNameOrdinals );

    for ( DWORD i{ 0 }; i < exportDir->NumberOfNames; ++i ) {
        auto name = reinterpret_cast<const char*>(
            reinterpret_cast<BYTE*>( m_modBase ) + names[i] );
        if ( _stricmp( name, m_funcName ) == 0 ) {
            return functions[ordinals[i]];
        }
    }
    return 0;
}