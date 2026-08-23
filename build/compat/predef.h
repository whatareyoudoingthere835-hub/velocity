// compat-предопределения до pch: phnt ждёт макросы/типы из MSVC SDK,
// которых нет в mingw-w64 заголовках
#pragma once

// phnt.h оборачивает все ntdll-декларации в EXTERN_C_START/END — в MSVC SDK
// они приходят из winnt.h новых версий; в mingw нет. Без extern "C" линкер
// не найдёт C-символы.
#ifndef EXTERN_C_START
#define EXTERN_C_START extern "C" {
#endif
#ifndef EXTERN_C_END
#define EXTERN_C_END }
#endif

// SAL-аннотации из MSVC, в mingw отсутствуют
#ifndef _Analysis_noreturn_
#define _Analysis_noreturn_
#endif
#ifndef _Enum_is_bitflag_
#define _Enum_is_bitflag_
#endif

// в апстримном phnt живёт в phnt_ntdef.h; в этой вендорной копии нет
#ifndef UFIELD_OFFSET
#define UFIELD_OFFSET(type, field) ((ULONG)__builtin_offsetof(type, field))
#endif
#ifndef UFIELD_SIZE
#define UFIELD_SIZE(type, field) (sizeof(((type*)0)->field))
#endif

#ifndef _Deref_post_count_
#define _Deref_post_count_(x)
#endif
#ifndef __callback
#define __callback
#endif
#ifndef DECLSPEC_ALLOCATOR
#define DECLSPEC_ALLOCATOR
#endif
#ifndef DECLSPEC_RESTRICT
#define DECLSPEC_RESTRICT
#endif
#ifndef _Post_valid_
#define _Post_valid_
#endif
#ifndef _Post_ptr_invalid_
#define _Post_ptr_invalid_
#endif
#ifndef _Post_invalid_
#define _Post_invalid_
#endif
#ifndef _Maybenull_
#define _Maybenull_
#endif
#ifndef _Notnull_
#define _Notnull_
#endif
#ifndef _Reserved_
#define _Reserved_
#endif
#ifndef _Analysis_assume_
#define _Analysis_assume_(x)
#endif

#ifndef _Deref_post_opt_count_
#define _Deref_post_opt_count_(x)
#endif
#ifndef _Deref_pre_bytecap_
#define _Deref_pre_bytecap_(x)
#endif
#ifndef _Deref_prepost_bytecount_
#define _Deref_prepost_bytecount_(x)
#endif

#include <cfloat> // FLT_MAX: misc.hpp ждёт его из MSVC-транзита
#include <thread>   // http.cpp: std::this_thread::sleep_for, в MSVC приезжал транзитом

// phnt: NTDDI_WIN10_FE гейт — при новом NTDDI_VERSION тип ждётся от SDK winnt.h,
// которого в mingw нет
#ifndef _RTL_SYSTEM_GLOBAL_DATA_ID_DEFINED
#define _RTL_SYSTEM_GLOBAL_DATA_ID_DEFINED
#define PHNT_COMPAT_NO_RTLSETHEAPINFO
typedef unsigned long RTL_SYSTEM_GLOBAL_DATA_ID;
#endif

// phnt: форвард-enum без определений (в C++ без ms-compat это ошибка).
// Код проекта эти типы не использует; пустые тела достаточно для ABI.
#ifndef _PHNT_COMPAT_EMPTY_ENUMS
#define _PHNT_COMPAT_EMPTY_ENUMS
typedef enum _USERTHREADINFOCLASS { } USERTHREADINFOCLASS, *PUSERTHREADINFOCLASS;
typedef enum _EVENT_INFO_CLASS { } EVENT_INFO_CLASS, *PEVENT_INFO_CLASS;
#endif

// STORAGE_RESERVE_ID — из winnt.h MSVC SDK (17763+), mingw не несёт, phnt использует в ntioapi
#ifndef _STORAGE_RESERVE_ID_DEFINED
#define _STORAGE_RESERVE_ID_DEFINED
typedef enum _STORAGE_RESERVE_ID {
    StorageReserveIdNone = 0,
    StorageReserveIdHard = 1,
    StorageReserveIdSoft = 2,
    StorageReserveIdMax = 3
} STORAGE_RESERVE_ID;
#endif

// phnt: полные определения вместо форвард-тайпдефов (ntexapi использует полями до ntioapi)
#ifndef PHNT_COMPAT_PRENUMS
#define PHNT_COMPAT_PRENUMS
typedef enum _INTERFACE_TYPE
{
    InterfaceTypeUndefined = -1,
    Internal = 0,
    Isa = 1,
    Eisa = 2,
    MicroChannel = 3,
    TurboChannel = 4,
    PCIBus = 5,
    VMEBus = 6,
    NuBus = 7,
    PCMCIABus = 8,
    CBus = 9,
    MPIBus = 10,
    MPSABus = 11,
    ProcessorInternal = 12,
    InternalPowerBus = 13,
    PNPISABus = 14,
    PNPBus = 15,
    MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

typedef enum _BUS_DATA_TYPE
{
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;
#endif
