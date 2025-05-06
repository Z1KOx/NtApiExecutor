#include "nt_api_executor.hpp"

// Constructor to load the NT function from a module (e.g., ntdll.dll)
NtApiExecutor::NtApiExecutor( const wchar_t* modName, const char* funcName )
    : m_modName( modName ), m_funcName( funcName )
{
    std::wcout << skCrypt( L"[*] Loading " ).decrypt()
               << modName << skCrypt( L"::" ).decrypt()
               << funcName << skCrypt( "\n" ).decrypt();

    m_modBase = getModuleBaseAddr();
    if ( !m_modBase ) {
        throw std::runtime_error( skCrypt( "Module not found" ).decrypt());
    }

    if ( !getExportRva() ) {
        throw std::runtime_error( skCrypt( "Function not found" ).decrypt());
    }
}

// Retrieves the base address of the loaded module
[[nodiscard]] PVOID 
    NtApiExecutor::getModuleBaseAddr() noexcept
{
    PPEB peb;

    // Get the PEB address
#ifdef _WIN64
    peb = reinterpret_cast<PPEB>( __readgsqword( 0x60 ) );
#else
    __asm {
        mov eax, fs: [0x30]
        mov peb, eax
    };
#endif
    if ( !peb || !peb->Ldr ) { return nullptr; }

    // Access the list of loaded modules from the PEB
    auto* head = &peb->Ldr->InMemoryOrderModuleList;
    auto* curr = head->Flink;

    for( curr = curr->Flink; curr != head; curr = curr->Flink )
    {
        auto* entry = CONTAINING_RECORD( curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );

        // Check if the module name matches the requested one
        if ( entry->BaseDllName.Buffer &&
            _wcsicmp( entry->BaseDllName.Buffer, m_modName ) == 0 ) 
        {
            std::wcout << skCrypt( L"[+] Found " ).decrypt()
                       << entry->BaseDllName.Buffer
                       << skCrypt( L" : 0x" ).decrypt()
                       << std::hex << reinterpret_cast<uintptr_t>( entry->DllBase )
                       << skCrypt( L"\n" ).decrypt();
            return entry->DllBase;
        }
    }
    return nullptr;
}

// Retrieves the export address of the function in the module
[[nodiscard]] DWORD 
    NtApiExecutor::getExportRva() noexcept 
{
    if ( !m_modBase ) { return 0; }

    // Access the DOS header of the module to find the NT headers
    auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>( m_modBase );
    auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<BYTE*>( m_modBase ) + dosHeader->e_lfanew );

    // Get the export directory from the NT headers
    auto exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
        reinterpret_cast<BYTE*>( m_modBase ) +
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress );

    // Get the arrays of names, functions, and ordinals from the export directory
    auto names = reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfNames );
    auto functions = reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfFunctions );
    auto ordinals = reinterpret_cast<WORD*>(
        reinterpret_cast<BYTE*>( m_modBase ) + exportDir->AddressOfNameOrdinals );

    // Iterate over the export names to find the function matching the requested name
    for ( DWORD i{ 0 }; i < exportDir->NumberOfNames; ++i ) {
        auto name = reinterpret_cast<const char*>(
            reinterpret_cast<BYTE*>( m_modBase ) + names[i] );
        if ( _stricmp( name, m_funcName ) == 0 ) {
            return functions[ordinals[i]];
        }
    }
    return 0;
}