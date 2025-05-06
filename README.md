# Introduction
This project provides a lightweight API to invoke NT functions directly from ```ntdll.dll``` without relying on ```LoadLibrary``` or ```GetProcAddress```. It facilitates seamless interaction with system-level functions such as ```NtCreateFile```.

## Usage Instructions
To use the API, follow these steps:
### 1. Declare the NT function you want to invoke
```cpp
typedef NTSTATUS( __stdcall* NtCreateFile_t )(
    PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
    PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
```
### 2. Initialize the ```NtApiExecutor``` to load the function from ```ntdll.dll```
```cpp
NtApiExecutor ntdll( L"ntdll.dll", "NtCreateFile" );
```
### 3. Resolve the function pointer
```cpp
const auto NtCreateFile = ntdll.resolveFunction<NtCreateFile_t>();
```
### 4. Prepare the parameters required for ```NtCreateFile```
```cpp
UNICODE_STRING uniStr = { 0 };
OBJECT_ATTRIBUTES objAttr = { 0 };
IO_STATUS_BLOCK ioStatus = { 0 };
HANDLE hFile = nullptr;
RtlInitUnicodeString( &uniStr, L"\\??\\C:\\test.txt" );
InitializeObjectAttributes( &objAttr, &uniStr, OBJ_CASE_INSENSITIVE, nullptr, nullptr );
```
### 5. Invoke the function using the API
```cpp
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
```
