# Introduction
This project provides a lightweight API to invoke NT functions directly from ```ntdll.dll``` without relying on ```LoadLibrary``` or ```GetProcAddress```. It allows seamless interaction with system-level functions such as ```NtAllocateVirtualMemory```.

## Usage Instructions
To use the API, follow these steps:
### 1. Declare the NT function you want to invoke
```cpp
typedef NTSTATUS( __stdcall* NtAllocateVirtualMemory_t )(
    HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG
);
```
### 2. Initialize the ```NtApiExecutor``` to load the function from ```ntdll.dll```
```cpp
NtApiExecutor ntdll( L"ntdll.dll", "NtAllocateVirtualMemory" );
```
### 3. Resolve the function pointer
```cpp
const auto NtAllocateVirtualMemory = ntdll.resolveFunction<NtAllocateVirtualMemory_t>();
```
### 4. Invoke the function using the API
```cpp
PVOID memory = nullptr;
SIZE_T size = 1024;
NtAllocateVirtualMemory( GetCurrentProcess(), &memory, 0, &size, 
                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
```
### Example Output
```
[+] Memory allocated at: 0000014ECA570000
```

# To clone this project with Git
```
git clone https://github.com/Z1KOx/NtExecutorAPI.git
```
