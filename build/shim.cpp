// shim: переходник MSVC-специфики velocity на mingw/llvm
// - DllMain: mingw CRT стартует сам (DllMainCRTStartup, глобальные ctors выполняются),
//   зовём entry() velocity; _CRT_INIT приходит из mingw dllcrt2
// - VMProtect SDK: реальный вызов .attach() закомментирован в entry.cpp,
//   функции нужны только чтобы линкер был доволен; возвращают failure
#include <pch/pch.hpp>

extern "C" int __stdcall entry( HMODULE module_handle, DWORD reason, LPVOID reserved );

extern "C" BOOL WINAPI DllMain( HINSTANCE module_handle, DWORD reason, LPVOID reserved )
{
    return entry( module_handle, reason, reserved ) != 0;
}

// --- VMProtect SDK stubs (объявления без dllimport, чтобы определить локально) ---
extern "C" {
    int __stdcall VMProtectSetSerialNumber( const char* serial );
    int __stdcall VMProtectGetSerialNumberState( );
    int __stdcall VMProtectGetSerialNumberData( void* data, int size );
    int __stdcall VMProtectBegin( const char* );
    int __stdcall VMProtectBeginVirtualization( const char* );
    int __stdcall VMProtectBeginMutation( const char* );
    int __stdcall VMProtectBeginUltra( const char* );
    int __stdcall VMProtectBeginVirtualizationLockByKey( const char* );
    int __stdcall VMProtectBeginUltraLockByKey( const char* );
    int __stdcall VMProtectEnd( );
    bool __stdcall VMProtectIsDebuggerPresent( );
    bool __stdcall VMProtectIsVirtualMachinePresent( );
    bool __stdcall VMProtectIsValidImageCRC( );
    bool __stdcall VMProtectIsProtected( );
    bool __stdcall VMProtectDump( void* );
    const char* __stdcall VMProtectDecryptStringA( const char* value );
    const wchar_t* __stdcall VMProtectDecryptStringW( const wchar_t* value );
    void* __stdcall VMProtectFreeString( const void* value );
    int __stdcall VMProtectSetUserData( const void*, int );
    int __stdcall VMProtectGetUserData( void*, int* );
    int __stdcall VMProtectGetUserID( );
    int __stdcall VMProtectGetUserDuration( );
}

int __stdcall VMProtectSetSerialNumber( const char* ) { return -1; }
int __stdcall VMProtectGetSerialNumberState( ) { return 0; }
int __stdcall VMProtectGetSerialNumberData( void*, int ) { return 0; }
int __stdcall VMProtectBegin( const char* ) { return 0; }
int __stdcall VMProtectBeginVirtualization( const char* ) { return 0; }
int __stdcall VMProtectBeginMutation( const char* ) { return 0; }
int __stdcall VMProtectBeginUltra( const char* ) { return 0; }
int __stdcall VMProtectBeginVirtualizationLockByKey( const char* ) { return 0; }
int __stdcall VMProtectBeginUltraLockByKey( const char* ) { return 0; }
int __stdcall VMProtectEnd( ) { return 0; }
bool __stdcall VMProtectIsDebuggerPresent( ) { return false; }
bool __stdcall VMProtectIsVirtualMachinePresent( ) { return false; }
bool __stdcall VMProtectIsValidImageCRC( ) { return true; }
bool __stdcall VMProtectIsProtected( ) { return true; }
bool __stdcall VMProtectDump( void* ) { return false; }
const char* __stdcall VMProtectDecryptStringA( const char* value ) { return value; }
const wchar_t* __stdcall VMProtectDecryptStringW( const wchar_t* value ) { return value; }
void* __stdcall VMProtectFreeString( const void* ) { return nullptr; }
int __stdcall VMProtectSetUserData( const void*, int ) { return 0; }
int __stdcall VMProtectGetUserData( void*, int* size ) { if ( size ) *size = 0; return 0; }
int __stdcall VMProtectGetUserID( ) { return 0; }
int __stdcall VMProtectGetUserDuration( ) { return 0; }

// --- MSVC /GS-обвязка: freetype.lib собран MSVC со stack cookies ---
// токен фиксированный: реальная рандомизация cookie тут не нужна, проверка no-op
extern "C" unsigned __int64 __security_cookie = 0x00002B99'2DDFA232ull;
extern "C" void __cdecl __security_check_cookie( unsigned __int64 ) { }
extern "C" void __cdecl __report_rangecheckfailure( )
{
    // выход вместо продолжения на повреждённых данных
    __builtin_trap( );
}
// GS-обработчик unwind для SEH-функций freetype: без проверки cookie
extern "C" __attribute__((sysv_abi)) void* __GSHandlerCheck( void* exception_record, unsigned long long establisher_frame, void* context_record, void* dispatcher_context )
{
    ( void ) exception_record; ( void ) establisher_frame; ( void ) context_record; ( void ) dispatcher_context;
    return ( void* ) 1; // ExceptionContinueSearch
}

// --- freetype.lib: MSVC-шные _setjmp/longjmp ---
// ручной x64: раскладка MSVC jmp_buf (rbx,rsp,rbp,rsi,rdi,r12..r15,rip), без SEH-unwind —
// пары setjmp/longjmp внутри freetype всегда простые
asm(
    ".text\n"
    ".globl _setjmp\n"
    "_setjmp:\n"                       // rcx = jmp_buf, rdx = frame ctx (игнор)
    "  movq %rbx, 0x00(%rcx)\n"
    "  movq %rsp, 0x08(%rcx)\n"
    "  movq %rbp, 0x10(%rcx)\n"
    "  movq %rsi, 0x18(%rcx)\n"
    "  movq %rdi, 0x20(%rcx)\n"
    "  movq %r12, 0x28(%rcx)\n"
    "  movq %r13, 0x30(%rcx)\n"
    "  movq %r14, 0x38(%rcx)\n"
    "  movq %r15, 0x40(%rcx)\n"
    "  movq (%rsp), %rax\n"
    "  movq %rax, 0x48(%rcx)\n"       // rip = адрес возврата
    "  xorl %eax, %eax\n"
    "  ret\n"
    ".globl longjmp\n"
    "longjmp:\n"                       // rcx = jmp_buf, edx = code
    "  movq 0x00(%rcx), %rbx\n"
    "  movq 0x10(%rcx), %rbp\n"
    "  movq 0x18(%rcx), %rsi\n"
    "  movq 0x20(%rcx), %rdi\n"
    "  movq 0x28(%rcx), %r12\n"
    "  movq 0x30(%rcx), %r13\n"
    "  movq 0x38(%rcx), %r14\n"
    "  movq 0x40(%rcx), %r15\n"
    "  movq 0x08(%rcx), %rsp\n"
    "  movl %edx, %eax\n"
    "  testl %eax, %eax\n"
    "  jne 1f\n"
    "  movl $1, %eax\n"
    "1:\n"
    "  jmpq *0x48(%rcx)\n"
);
