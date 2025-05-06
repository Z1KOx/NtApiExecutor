#include "nt_api_executor.hpp"

// Function pointer for NtAllocateVirtualMemory
typedef NTSTATUS(__stdcall* NtAllocateVirtualMemory_t)(
    HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG
);

int main()
{
    try
    {
        NtApiExecutor ntdll( L"ntdll.dll", "NtAllocateVirtualMemory" );
        const auto NtAllocateVirtualMemory = ntdll.resolveFunction<NtAllocateVirtualMemory_t>();
        if ( !NtAllocateVirtualMemory ) { throw std::runtime_error( "NtAllocateVirtualMemory not resolved." ); }

        PVOID memory = nullptr;
        SIZE_T size = 1024;

        // Allocate memory in the current process
        NtAllocateVirtualMemory( GetCurrentProcess(), &memory, 0, &size, 
                                 MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
        std::cout << "[+] Memory allocated at: " << memory << '\n';
    }
    catch ( const std::exception& e ) {
        std::cout << e.what() << '\n';
    }

    return 0;
}