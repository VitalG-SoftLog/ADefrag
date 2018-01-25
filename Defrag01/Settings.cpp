/*
    Defrag Server Settings 

    Module name:

        Settings.cpp

    Abstract:

        Defrag Server Settings module.
        Defines settings internal data, functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Settings.cpp,v 1.5 2009/12/25 15:58:15 dimas Exp $
    $Log: Settings.cpp,v $
    Revision 1.5  2009/12/25 15:58:15  dimas
    Priority control implemented

    Revision 1.4  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.3  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.2  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/
    
#include "stdafx.h"


//
// Contains TRUE if Settings subsystem is initialized. Local to this module.
//
static volatile BOOLEAN             SettingsInitialized = FALSE;

//
// Service Regestry path.
//
UNICODE_STRING  SettingsRegistryPath = { 0 };

UNICODE_STRING  JobsRegistryPath = { 0 };

//
// Define settings.
//
DFRG_SETTINGS DfrgSettings = { 0 };

//
// =============================
//      DWORD Setting type
// =============================
//
// Save DWORD Value (in Registry) method
//
ULONG
DfrgRegWriteDword(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    UNICODE_STRING      ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };
    ULONG               Value = *(PULONG)SettingAddress;

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE | OBJ_OPENIF, // | OBJ_KERNEL_HANDLE
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        Status = NtSetValueKey(
            KeyHandle,
            &ValueNameString,
            0,
            REG_DWORD,
            &Value,
            sizeof(Value));

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

#ifdef _DEBUG
    if (!NT_SUCCESS(Status))
    {
        //swprintf( Mess, L"DfrgMoveClusters. NtFsControlFile" );
        DislpayErrorMessage( L"DfrgRegWriteDword. ", Status, ERR_NT_STATUS );
    }
#endif

    return Status;
} // end of DfrgRegWriteDword

//
// Read DWORD Value (form Registry) method
//
ULONG
DfrgRegReadDword(
    IN  PWCHAR   ValueName,
    OUT PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    PKEY_VALUE_FULL_INFORMATION ValueInfo = NULL;
    UNICODE_STRING      ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        ULONG Size = 0;
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            NULL,
            NULL,
            &Size);

        if (    (!NT_SUCCESS(Status))
            &&  (Status!=STATUS_BUFFER_TOO_SMALL)
            &&  (Status!=STATUS_BUFFER_OVERFLOW))
        {
            __leave;
        }
        
        ValueInfo = (PKEY_VALUE_FULL_INFORMATION) malloc( Size );
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            ValueInfo,
            Size,
            &Size);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }
        
        //ASSERT(ValueInfo->DataLength==sizeof(DWORD));
        //ASSERT(ValueInfo->Type==REG_DWORD);

        *(PULONG)(SettingAddress) = *(PULONG)((PCHAR)(ValueInfo) + ValueInfo->DataOffset);

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    if (ValueInfo)
    {
        free(ValueInfo);
    }

    return Status;

} // end of DfrgRegReadDword

//
// Set/Get DWORD Value methods definitions
//
// Set DWORD Value methods definitions
//
ULONG
DfrgSetDword(
    IN OUT PVOID           SettingAddress,
    IN     PVOID           SettingBuffer,
    IN OUT PULONG          Size)
{
    UNREFERENCED_PARAMETER( Size );

    int                 Result      = NO_ERROR;
    PULONG      Dword       = (PULONG)SettingAddress;
    PULONG      ValueToSet  = (PULONG)SettingBuffer;
    
    __try
    {
        *Dword = *ValueToSet;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

}

//
// Get DWORD Value methods definitions
//
ULONG
DfrgGetDword(
    IN OUT PVOID           SettingAddress,
    IN     PVOID           SettingBuffer,
    IN OUT PULONG           Size)
{
    int         Result      = NO_ERROR;
    PULONG      Dword       = (PULONG)SettingAddress;
    PULONG      BufferToGet = (PULONG)SettingBuffer;
    
    __try
    {
        //
        // If caller specified too small buffer set error code
        //
        if (*Size < sizeof(DWORD) && BufferToGet)
        {
            Result = ERROR_BUFFER_OVERFLOW;
        }

        //
        // If callers specified too small buffer or if caller
        // just wants to know the size, set it
        //
        if (*Size < sizeof(DWORD) || !BufferToGet)
        {
            *Size = sizeof(DWORD);
            __leave;
        }

        *BufferToGet = *Dword;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

}

//
// =============================
//      STRING Setting type
// =============================
//
// Save String Value (in Registry) method
//
ULONG
DfrgRegWriteString(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    PUNICODE_STRING     String = (PUNICODE_STRING)SettingAddress;
    UNICODE_STRING      ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        //
        // Get size of the data
        //
        SIZE_T DataSize = String->Length;

        Status = NtSetValueKey(
            KeyHandle,
            &ValueNameString,
            0,
            REG_SZ,
            String->Buffer,
            (ULONG)DataSize);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    return Status;

} // end of DfrgRegWriteString

//
// Read String Value (from Registry) method
//
ULONG
DfrgRegReadString(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    PKEY_VALUE_FULL_INFORMATION ValueInfo = NULL;
    PUNICODE_STRING     Value = (PUNICODE_STRING)SettingAddress;
    UNICODE_STRING      ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        ULONG Size = 0;
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            NULL,
            NULL,
            &Size);

        if ( (!NT_SUCCESS(Status))
            &&  (Status!=STATUS_BUFFER_TOO_SMALL)
            &&  (Status!=STATUS_BUFFER_OVERFLOW) )
        {
            __leave;
        }

        if ( !Size )
        {
            //
            // Set error to clear string
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }

        ValueInfo = (PKEY_VALUE_FULL_INFORMATION) malloc( Size );
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            ValueInfo,
            Size,
            &Size);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }
        
        //ASSERT(ValueInfo->Type==REG_SZ);
        PWCHAR ValuePtr = (PWCHAR)((PCHAR)(ValueInfo) + ValueInfo->DataOffset);

        if (!UniAllocateAtLeast(Value, ValueInfo->DataLength/sizeof(WCHAR)+1))
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        Value->Length = (USHORT)ValueInfo->DataLength;
        //
        // Round length
        //
        Value->Length = Value->Length & ~1;

        memcpy(Value->Buffer, ValuePtr, ValueInfo->DataLength);

        //
        // Protection against malformed registry value.
        // Leave/set one trailing zero byte to fit our UNICODE_STRING requirements (see Unicode.*).
        //
        size_t length = UniGetLen(Value);
        Value->Buffer[length] = L'\0';
        while ( length && Value->Buffer[length-1] == L'\0' )
        {
            length--;
            Value->Length -= sizeof(WCHAR);
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    if (ValueInfo)
    {
        free(ValueInfo);
    }

    if ( !NT_SUCCESS( Status ) &&
        STATUS_INSUFFICIENT_RESOURCES != Status )
    {
        //
        // Set empty string if could not get value
        //
        //UniFreeUnicodeString( Value );
        Status = STATUS_SUCCESS;
        if ( !UniAllocateAtLeast( Value, sizeof(WCHAR) ) )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Value->Length = 0;
            *(Value->Buffer) = L'\0';
        }
    }

    return Status;

} // end of DfrgRegReadString

//
// Set/Get String Value methods definitions
//
// Get String Value methods definitions
//
ULONG
DfrgSetString(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size )
{
    UNREFERENCED_PARAMETER( Size );

    int                     Result      = NO_ERROR;
    PUNICODE_STRING String      = (PUNICODE_STRING)SettingAddress;
    PWCHAR          ValueToSet  = (PWCHAR)SettingBuffer;
    
    __try
    {
        SIZE_T  Length = wcslen(ValueToSet);

        if (!UniAllocateAtLeast(String, Length+1))
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        memcpy(String->Buffer, ValueToSet, Length*sizeof(WCHAR));
        String->Buffer[Length] = L'\0';
        String->Length = (USHORT)Length*sizeof(WCHAR);

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

}

//
// Set String Value methods definitions
//
ULONG
DfrgGetString(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size )
{
    int                     Result      = NO_ERROR;
    PUNICODE_STRING String      = (PUNICODE_STRING)SettingAddress;
    PWCHAR          BufferToGet = (PWCHAR)SettingBuffer;
    
    __try
    {
        //
        // If caller specified too small buffer set error code
        //
        if (*Size < String->Length + sizeof(WCHAR) && BufferToGet)
        {
            Result = ERROR_BUFFER_OVERFLOW;
        }

        //
        // If callers specified too small buffer or if caller
        // just wants to know the size, set it
        //
        if (*Size < String->Length + sizeof(WCHAR) || !BufferToGet)
        {
            *Size = String->Length + sizeof(WCHAR);
            __leave;
        }

        memcpy(BufferToGet, String->Buffer, String->Length);
        BufferToGet[UniGetLen(String)] = L'\0';
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

}

//
// =============================
//      MULTY_SZ Setting type
// =============================
//
// Save Multi String Value (in Registry) method
//
ULONG
DfrgRegWriteList(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    PUNICODE_STRING     List = (PUNICODE_STRING)SettingAddress;
    UNICODE_STRING ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        //
        // Get size of the data
        //
        SIZE_T DataSize = List->Length;


        Status = NtSetValueKey(
            KeyHandle,
            &ValueNameString,
            0,
            REG_MULTI_SZ,
            List->Buffer,
            (ULONG)DataSize);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    return Status;

} // end of DfrgRegWriteList

//
// Read Multi String Value (from Registry) method
//
ULONG
DfrgRegReadList(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    PKEY_VALUE_FULL_INFORMATION ValueInfo = NULL;
    PUNICODE_STRING     Value = (PUNICODE_STRING)SettingAddress;
    UNICODE_STRING ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &SettingsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        ULONG Size = 0;
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            NULL,
            NULL,
            &Size);

        if ( (!NT_SUCCESS(Status))
            &&  (Status!=STATUS_BUFFER_TOO_SMALL)
            &&  (Status!=STATUS_BUFFER_OVERFLOW) )
        {
            __leave;
        }

        if ( !Size )
        {
            //
            // Set error to clear list
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }
       
        ValueInfo = (PKEY_VALUE_FULL_INFORMATION) malloc( Size );
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            ValueInfo,
            Size,
            &Size);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }
        
        if ( ValueInfo->Type != REG_MULTI_SZ )
        {
            //
            // Set error to clear list
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }

        PWCHAR ValuePtr = (PWCHAR)((PCHAR)(ValueInfo) + ValueInfo->DataOffset);

        if (!UniAllocateAtLeast(Value, ValueInfo->DataLength/sizeof(WCHAR) + 2))
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        Value->Length = (USHORT)ValueInfo->DataLength;

        //
        // Round length
        //
        Value->Length = Value->Length & ~1;

        memcpy(Value->Buffer, ValuePtr, Value->Length);

        //
        // Protection against malformed registry value.
        // Leave/set two trailing zero bytes to fit our requirements 
        // for UNICODE_STRING containing MULTI_SZ list (see Unicode.*).
        //
        size_t length = UniGetLen(Value);
        Value->Buffer[length]   = L'\0';
        Value->Buffer[length+1] = L'\0';
        length += 2;
        Value->Length += 2*sizeof(WCHAR);
        while ( length-2 && Value->Buffer[length-3] == L'\0' )
        {
            length--;
            Value->Length -= sizeof(WCHAR);
        }

        if ( Value->Length <= 2*sizeof(WCHAR) )
        {
            Value->Length = sizeof(WCHAR);
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    if (ValueInfo)
    {
        free(ValueInfo);
    }

    if ( !NT_SUCCESS( Status ) &&
        STATUS_INSUFFICIENT_RESOURCES != Status )
    {
        //
        // Set empty list if could not get value
        //
        //MlsDelete( Value );
        Status = STATUS_SUCCESS;
        if ( !UniAllocateAtLeast( Value, sizeof(WCHAR)*2 ) )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Value->Length = sizeof(WCHAR);
            *(Value->Buffer)   = L'\0';
            *(Value->Buffer+1) = L'\0';
        }
    }

    return Status;

} // end of DfrgRegReadList

//
// List (Multi String Value) Add/Remove/Get (TYPE_METHOD) methods definitions
//
// Merge ListItems MULTI_SZ-list to List MULTI_SZ-list
//
ULONG
DfrgAddItemsToList(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size)
{
    UNREFERENCED_PARAMETER( Size );

    PUNICODE_STRING List      = (PUNICODE_STRING)SettingAddress;
    PWCHAR          ListItems = (PWCHAR)SettingBuffer;

    int             Result    = NO_ERROR;

    PWCHAR          TempBuffer = NULL;
    PWCHAR          ListItemsCopy;
    UNICODE_STRING  ConvertedString = { 0 };

    __try
    {
        if ( !SettingBuffer )
        {
            Result    = ERROR_INVALID_PARAMETER;
            __leave;
        }

        TempBuffer = MlsDuplicateList(ListItems);
        if (!TempBuffer)
        {
            __leave;
        }

        ListItemsCopy = TempBuffer;

        //
        // Iterate through all items and add each to list
        //
        while (*ListItemsCopy)
        {
            PWCHAR CurrentItem = ListItemsCopy;
/*
            if (List == &DfrgSettings.ExcludedDirs)
            {
                UNICODE_STRING SourceString;

                RtlInitUnicodeString(&SourceString, ListItemsCopy);

                if (PathCreateWindowsForm(&ConvertedString, &SourceString, FALSE))
                {
                    CurrentItem = ConvertedString.Buffer;
                }
            }
*/
            if (!MlsIsItemPresent(List->Buffer, CurrentItem, NULL))
            {
                if (!MlsAppend(List, CurrentItem))
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }
            }
            ListItemsCopy += wcslen(ListItemsCopy) + 1;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    if (TempBuffer)
    {
        free(TempBuffer);
    }

    UniFreeUnicodeString( &ConvertedString );

    return Result;

} // end of DfrgAddItemsToList

//
// Remove ListItems MULTI_SZ-list from List MULTI_SZ-list
//
ULONG
DfrgRemoveItemsFromList(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size)
{
    UNREFERENCED_PARAMETER( Size );

    PUNICODE_STRING List      = (PUNICODE_STRING)SettingAddress;
    PWCHAR          ListItems = (PWCHAR)SettingBuffer;
    
    int             Result    = NO_ERROR;

    PWCHAR          TempBuffer = NULL;
    PWCHAR          ListItemsCopy;

    __try
    {
        if ( !SettingBuffer )
        {
            //
            // NULL means "remove all entries"
            //
            List->Buffer[0] = L'\0';
            List->Length = 2;
            __leave;
        }

        TempBuffer = MlsDuplicateList(ListItems);
        if (!TempBuffer)
        {
            __leave;
        }

        ListItemsCopy = TempBuffer;

        //
        // Iterate through all items and remove each from list
        //
        while (*ListItemsCopy)
        {
            SIZE_T ListItemLength;
            PWCHAR ListItem = MlsIsItemPresent(List->Buffer, ListItemsCopy, &ListItemLength);

            if (ListItem)
            {
                //
                // Shift list contents down, erasing current item
                //
                memmove(ListItem, ListItem + ListItemLength + 1, (UniGetLen(List) - ListItemLength - 1 - (ListItem - List->Buffer) )*sizeof(WCHAR));
                List->Length -= (USHORT)(ListItemLength + 1)*sizeof(WCHAR);
            }
            else
            {
                //
                // Move to next item only if we removed all instances of current item
                //
                ListItemsCopy += wcslen(ListItemsCopy) + 1;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    if (TempBuffer)
    {
        free(TempBuffer);
    }

    return Result;

} // end of DfrgRemoveItemsFromList

//
// Get List
//
ULONG
DfrgGetList(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           ListSize)
{
    PUNICODE_STRING List       = (PUNICODE_STRING)SettingAddress;
    PWCHAR          ListExport = (PWCHAR)SettingBuffer;

    int               Result = NO_ERROR;

    __try
    {
        //
        // If caller specified too small buffer set error code
        //
        if ( *ListSize && *ListSize < UniGetLen(List) && ListExport )
        {
            Result = ERROR_BUFFER_OVERFLOW;
        }

        //
        // If callers specified too small buffer or if caller
        // just wants to know list size, set it
        //
        if ( !(*ListSize) || !ListExport )
        {
            *ListSize = (ULONG)(UniGetLen(List) * sizeof(WCHAR));
            __leave;
        }

        memcpy(ListExport, List->Buffer, List->Length);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

} // end of DfrgGetList

//
// List SetRoutine
//
int              
DfrgSetFilteringList(
    IN SETTING_ID SettingId,
    IN PVOID      SettingBuffer)
{
    PUNICODE_STRING List = (PUNICODE_STRING)SettingBuffer;

    switch (SettingId)
    {
        case ExcludedFilesId:
            //DfrgSetExcludeArray( List );

            if ( !IsEmptyUnicodeString( List ) )
            {
                PWCHAR ExcludePathName = List->Buffer;

                //
                // Iterate through all the items and find matching one
                //
                while ( *ExcludePathName )
                {
                    _wcsupr( ExcludePathName );

                    ExcludePathName = ExcludePathName + wcslen(ExcludePathName) + 1;
                }
            }

            break;

        default:
            break;
    }

    return NO_ERROR;

} // end of DfrgSetFilteringList

//
// =============================
//      JOB Setting type
// =============================
//

DWORD       JobsNum = 0;

void
FreeSheduledJobList( void )
{
    PSHEDULED_JOB_ITEM   Job = DfrgSettings.SheduledJobs, NextJob;

    while( Job )
    {
        NextJob = Job->NextJob;
        free( Job );
        Job = NextJob;
    }

    DfrgSettings.SheduledJobs = NULL;
    JobsNum = 0;

    SetShedulerEvent();

} // end of FreeSheduledJobList


void
DeleteSheduledJob(
    PSHEDULED_JOB_ITEM   Job )
{
    PSHEDULED_JOB_ITEM   CurrentJob = Job, NextJob, PrevJob;

    if ( CurrentJob )
    {
        NextJob = CurrentJob->NextJob;
        PrevJob = CurrentJob->PrevJob;

        free( CurrentJob );

        if ( PrevJob )
        {
            PrevJob->NextJob = NextJob;
        }
        else
        {
            DfrgSettings.SheduledJobs = NextJob;
        }

        if ( NextJob )
        {
            NextJob->PrevJob = PrevJob;
        }

        JobsNum--;

        SetShedulerEvent();
    }
} // end of DeleteSheduledJob


PSHEDULED_JOB_ITEM
FindSheduledJob( 
    PDFRG_SHEDULED_JOB   JobToFind )
{
    PSHEDULED_JOB_ITEM   Job = DfrgSettings.SheduledJobs, FindJob = NULL;

    while( Job )
    {
        if ( !wcscmp( JobToFind->Name, Job->Job.Name ) )
        {
            FindJob = Job;
            break;
        }
        Job = Job->NextJob;
    }

    return FindJob;

} // end of FindSheduledJob


int
AddSheduledJob( 
    PDFRG_SHEDULED_JOB   NewJob )
{
    DWORD           Result = NO_ERROR;
    PSHEDULED_JOB_ITEM   JobItem = NULL;


    if ( NewJob )
    {
        //
        // Allocate memory
        //
        JobItem = (PSHEDULED_JOB_ITEM) malloc( sizeof(SHEDULED_JOB_ITEM) );
        if ( !JobItem )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            return Result;
        }

        //
        // Copy Body
        //
        memcpy( &(JobItem->Job), NewJob, sizeof(DFRG_SHEDULED_JOB) );
        JobItem->PrevJob = NULL;
        JobItem->NextJob = NULL;

        //
        // Find last job in list to attach new job to
        //
        PSHEDULED_JOB_ITEM       LastJob = DfrgSettings.SheduledJobs; // *((PSHEDULED_JOB_ITEM*)SettingAddress);

        if ( !LastJob )
        {
            //*((PSHEDULED_JOB_ITEM*)SettingAddress) = JobItem;
            DfrgSettings.SheduledJobs = JobItem;
        }
        else
        {
            while( LastJob->NextJob )
            {
                LastJob = LastJob->NextJob;
            }

            LastJob->NextJob = JobItem;
            JobItem->PrevJob = LastJob;
        }

        JobsNum++;

#ifdef _DEBUG
        OutMessage( L"   ==>ShedulerTask Added=> Update Tasks" );
#endif

        SetShedulerEvent();
    }


    return Result;

} // end of AddSheduledJob

//
// Save Job (in Registry) method
//
ULONG
DfrgRegWriteJob(
    IN PWCHAR   ValueName,
    IN PVOID    SettingAddress)
{
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;

    OBJECT_ATTRIBUTES   ObjectAttr;
    HANDLE              KeyHandle = NULL;
    UNICODE_STRING      ValueNameString = { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    PDFRG_SHEDULED_JOB  JobsArray = NULL;


    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &JobsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        //
        // Build Job array. 
        // Culculate size, allocate and fill.
        //
        SIZE_T DataSize = JobsNum * sizeof(DFRG_SHEDULED_JOB);

        JobsArray = (PDFRG_SHEDULED_JOB) malloc(DataSize);
        if ( !JobsArray )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        PSHEDULED_JOB_ITEM   JobItem = *((PSHEDULED_JOB_ITEM*)SettingAddress); //DfrgSettings.SheduledJobs;
        DWORD           Count = 0;

        while( JobItem )
        {
            memcpy( JobsArray+Count, &(JobItem->Job), sizeof(DFRG_SHEDULED_JOB) );
            Count++;
            JobItem = JobItem->NextJob;
        }

        Status = NtSetValueKey(
            KeyHandle,
            &ValueNameString,
            0,
            REG_BINARY,
            JobsArray,
            (ULONG)DataSize);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    if ( JobsArray )
    {
        free( JobsArray );
    }

    return Status;

} // end of DfrgRegWriteJob

//
// Read Job (from Registry) method
//
ULONG
DfrgRegReadJob(
    IN PWCHAR       ValueName,
    IN PVOID        SettingAddress)
{
    NTSTATUS                        Status = STATUS_INVALID_PARAMETER;

    OBJECT_ATTRIBUTES               ObjectAttr;
    HANDLE                          KeyHandle = NULL;
    PKEY_VALUE_FULL_INFORMATION     ValueInfo = NULL;
    UNICODE_STRING                  ValueNameString = 
                                        { (USHORT)wcslen(ValueName)*sizeof(WCHAR), (USHORT)wcslen(ValueName)*sizeof(WCHAR), ValueName };

    PDFRG_SHEDULED_JOB              Job = NULL;


    __try
    {
        InitializeObjectAttributes(
            &ObjectAttr,
            &JobsRegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtCreateKey(
            &KeyHandle,
            KEY_ALL_ACCESS,
            &ObjectAttr,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

        ULONG Size = 0;
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            NULL,
            NULL,
            &Size);

        if ( (!NT_SUCCESS(Status))
            &&  (Status!=STATUS_BUFFER_TOO_SMALL)
            &&  (Status!=STATUS_BUFFER_OVERFLOW) )
        {
            __leave;
        }

        if ( !Size )
        {
            //
            // Set error to clear list
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }
       
        ValueInfo = (PKEY_VALUE_FULL_INFORMATION) malloc( Size );
        Status = NtQueryValueKey(
            KeyHandle,
            &ValueNameString,
            KeyValueFullInformation,
            ValueInfo,
            Size,
            &Size);

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }
        
        if ( ValueInfo->Type != REG_BINARY )
        {
            //
            // Set error to clear list
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }

        DWORD   Count = ValueInfo->DataLength / sizeof(DFRG_SHEDULED_JOB);

        //
        // Protection against malformed registry value
        //
        if ( !Count )
        {
            //
            // Set error to clear list
            //
            Status = STATUS_BUFFER_OVERFLOW;
            __leave;
        }

        Job = (PDFRG_SHEDULED_JOB)((PCHAR)(ValueInfo) + ValueInfo->DataOffset);

        for( DWORD i=0; i<Count; i++ )
        {
            if ( NO_ERROR != AddSheduledJob( Job+i ) )
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }
        }

        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (KeyHandle)
    {
        NtClose(KeyHandle);
    }

    if (ValueInfo)
    {
        free(ValueInfo);
    }

    if ( !NT_SUCCESS( Status ) &&
        STATUS_INSUFFICIENT_RESOURCES != Status )
    {
        //
        // Set empty list if could not get value
        //
        FreeSheduledJobList();
    }

    return Status;

} // end of DfrgRegReadJob

//
// Job Add/Remove/Get (TYPE_METHOD) methods definitions
//
// Add Job to List 
//
ULONG
DfrgAddJob(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size )
{
    //UNREFERENCED_PARAMETER( Size );

    int             Result    = NO_ERROR;


    __try
    {
        if ( !SettingBuffer )
        {
            Result    = ERROR_INVALID_PARAMETER;
            __leave;
        }

        PDFRG_SHEDULED_JOB          InJob = NULL;
        PSHEDULED_JOB_ITEM          JobItem = NULL;

        DWORD   InJobsCnt =  *Size / sizeof(DFRG_SHEDULED_JOB);

        for( DWORD i=0; i<InJobsCnt; i++ )
        {
            InJob = (PDFRG_SHEDULED_JOB)SettingBuffer + i;

            //
            // Find existing Job with the same name / id
            //
            JobItem = FindSheduledJob( InJob );

            if ( JobItem )
            {
                //
                // Set Job parameters
                //
                memcpy( &(JobItem->Job), InJob, sizeof(DFRG_SHEDULED_JOB) );
                SetShedulerEvent();
            }
            else
            {
                //
                // Add new Job 
                //
                Result = AddSheduledJob( InJob );
                if ( NO_ERROR != Result )
                {
                    __leave;
                }
            }
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

} // end of DfrgAddJob

//
// Remove Job from List 
//
ULONG
DfrgRemoveJob(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           Size)
{
    UNREFERENCED_PARAMETER( Size );

    int                     Result    = NO_ERROR;

    __try
    {
        if ( !SettingBuffer || !Size )
        {
            //
            // NULL means "remove all entries"
            //
            FreeSheduledJobList();
            __leave;
        }

        PSHEDULED_JOB_ITEM   JobItem = FindSheduledJob( (PDFRG_SHEDULED_JOB)SettingBuffer );
        if ( JobItem )
        {
            DeleteSheduledJob( JobItem );
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

} // end of DfrgRemoveJob

//
// Get Job list
//
ULONG
DfrgGetJobList(
    IN OUT PVOID            SettingAddress,
    IN     PVOID            SettingBuffer,
    IN OUT PULONG           ListSize)
{
    int                 Result = NO_ERROR;

    __try
    {
        ULONG           Size = JobsNum * sizeof(DFRG_SHEDULED_JOB);

        //
        // If caller specified too small buffer set error code
        //
        if ( *ListSize && *ListSize < Size && SettingBuffer )
        {
            Result = ERROR_BUFFER_OVERFLOW;
        }

        //
        // If callers specified too small buffer or if caller
        // just wants to know list size, set it
        //
        if ( !(*ListSize) || !SettingBuffer )
        {
            *ListSize = Size;
            __leave;
        }

        PSHEDULED_JOB_ITEM      JobItem = *((PSHEDULED_JOB_ITEM*)SettingAddress); //DfrgSettings.SheduledJobs;
        DWORD                   Count = 0;

        *ListSize = 0;

        while( JobItem )
        {
            memcpy( (PDFRG_SHEDULED_JOB)SettingBuffer + Count, &(JobItem->Job), sizeof(DFRG_SHEDULED_JOB) );
            JobItem = JobItem->NextJob;
            Count++;
            (*ListSize) += sizeof(DFRG_SHEDULED_JOB);
        }

        //ASSERT( *ListSize == Size );

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
    }

    return Result;

} // end of DfrgGetJobList

//
// Job SetRoutine
//
int              
DfrgSetJobList(
    IN SETTING_ID SettingId,
    IN PVOID      SettingBuffer)
{
    PUNICODE_STRING List = (PUNICODE_STRING)SettingBuffer;


    return NO_ERROR;

} // end of DfrgSetJobList


//
// Other settings SetRoutines
//
int              
DfrgSetLoggingDepth(
    IN SETTING_ID SettingId,
    IN PVOID      SettingBuffer)
{
    UNREFERENCED_PARAMETER( SettingBuffer );
    UNREFERENCED_PARAMETER( SettingId );

    if ( DfrgSettings.LoggingDepth > DfrgLoggingDepthMax )
    {
        DfrgSettings.LoggingDepth = DfrgLoggingDepthDef;
    }

    return NO_ERROR;

} // end of DfrgSetLoggingDepth


int              
DfrgSetCpuPriority(
    IN SETTING_ID SettingId,
    IN PVOID      SettingBuffer)
{
    UNREFERENCED_PARAMETER( SettingBuffer );
    UNREFERENCED_PARAMETER( SettingId );


#ifndef DEFRAG_NATIVE_APP

    ULONG   CpuPriority     = DfrgSettings.CpuPriority;
    ULONG   PriorityClass   = NORMAL_PRIORITY_CLASS; // 0x20

#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
#endif

    switch ( CpuPriority )
    {
    case DfgCpuPriorityIdle:

        PriorityClass = IDLE_PRIORITY_CLASS; //0x40

        break;

    case DfgCpuPriorityTurbo:

        PriorityClass = HIGH_PRIORITY_CLASS; // 0x80

        break;

    case DfgCpuPriorityNormal:

        PriorityClass = NORMAL_PRIORITY_CLASS; // 0x20

    default:

        CpuPriority = DfgCpuPriorityNormal;
        PriorityClass = NORMAL_PRIORITY_CLASS;

        //
        // Restore default value
        //
        DfrgSettings.CpuPriority = CpuPriority;

        break;
    }

    //
    // Set process priority
    //
    if ( DfrgSettings.AdjustCpuPriority )
    {
#ifdef _DEBUG
        swprintf( Mess, L"   # DfrgSetCpuPriority: %d ==> %08X", 
            CpuPriority, PriorityClass );
        OutMessage( Mess );
#endif

        SetPriorityClass( GetCurrentProcess(), PriorityClass );
    }
    else
    {
#ifdef _DEBUG
        swprintf( Mess, L"   # DfrgSetCpuPriority: %d X=> %08X", 
            CpuPriority, NORMAL_PRIORITY_CLASS );
        OutMessage( Mess );
#endif

        SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS );
    }

#endif // DEFRAG_NATIVE_APP


    return NO_ERROR;

} // end of DfrgSetCpuPriority




TYPE_VTABLE_ENTRY   TypeVtable[TYPE_MAX_ID]   =
{
    //
    // DWORD Value
    //
    {
        {
            DfrgSetDword,
            NULL,
            DfrgGetDword
        },
        DfrgRegReadDword,
        DfrgRegWriteDword
    },

    //
    // String Value
    //
    {
        {
            DfrgSetString,
            NULL,
            DfrgGetString
        },
        DfrgRegReadString,
        DfrgRegWriteString
    },

    //
    // Multi String Value
    //
    { 
        {
            DfrgAddItemsToList,
            DfrgRemoveItemsFromList,
            DfrgGetList 
        },
        DfrgRegReadList,
        DfrgRegWriteList
    },

    //
    // Job Value
    //
    {
        {
            DfrgAddJob,
            DfrgRemoveJob,
            DfrgGetJobList
        },
        DfrgRegReadJob,
        DfrgRegWriteJob
    },
};

#ifdef _DEBUG

PWCHAR 
GetOpString( 
    ULONG   OpId )
{
    switch ( OpId )
    {
        case OP_SET:    return L"Set/Add";
        case OP_GET:    return L"Get";
        case OP_REMOVE: return L"Remove";
        case OP_REMOVE_ALL: return L"Remove All";
    }

    return L"[invalid Op]";
}

#endif

//
// Set settings meta info table.
//
#undef SETTING
#define SETTING(Name, Type, Context, SetRoutine, DefaultValue)                        \
{   Name##Id, L#Name, Type, &DfrgSettings.Name, Context, SetRoutine, (PVOID)DefaultValue  },

SETTING_META_INFO   SettingsTable[] =
{
    #include "SettingsTable.h"
    {   0   }
};


//
// Settings Control Function
// 
int              
DfrgApiSettingOp(
    IN     ULONG        SettingId,
    IN     ULONG        OpId,
    IN OUT PVOID        SettingBuffer,
    IN OUT PULONG       SettingBufferSize )
{
    int             Result = NO_ERROR;
    SETTING_TYPE    SettingType;

    if (OpId >= OP_MAX_ID)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (SettingId >= SettingMaxId)
    {
        return ERROR_INVALID_PARAMETER;
    }

    SettingType = SettingsTable[SettingId].Type;

    LockSettings(&DfrgSettings.SyncObject);

    PVOID Address = SettingsTable[SettingId].Address;

#ifdef _DEBUG
if ( OP_GET != OpId )
{
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"   --- Operation: %ls for setting %ls", 
        GetOpString(OpId),  SettingsTable[SettingId].Name );
    OutMessage( Mess );

    if ( TYPE_JOB != SettingsTable[SettingId].Type )
    {
        switch( SettingsTable[SettingId].Type )
        {
        case TYPE_DWORD:
            swprintf( Mess, L"      - (cur %04X", *(DWORD*)(Address) );
            break;
        case TYPE_STRING:
        case TYPE_MULTI_SZ:
            swprintf( Mess, L"      - (cur '%ls'", ((PUNICODE_STRING)Address)->Buffer );
            break;
        case TYPE_JOB:
            break;
        }

        if ( SettingBuffer && (OP_SET == OpId || OP_REMOVE == OpId ) )
        {
            switch( SettingsTable[SettingId].Type )
            {
            case TYPE_DWORD:
                swprintf( Mess+wcslen(Mess), L", new %04X)", *(DWORD*)(SettingBuffer) );
                break;
            case TYPE_STRING:
            case TYPE_MULTI_SZ:
                swprintf( Mess+wcslen(Mess), L", new '%ls')", (PWCHAR)(SettingBuffer) );
                break;
            case TYPE_JOB:
                break;
            }
        }
        else
        {
            wcscat( Mess, L")" );
        }
        OutMessage( Mess );
    }
}
#endif

    PTYPE_METHOD Method = TypeVtable[SettingType].Methods[OpId];

    if (Method)
    {
        ULONG OldValue = 0;

        if (SettingType == TYPE_DWORD && OpId == OP_SET)
        {
            //
            // Save old value to restore it if user passed invalid parameter
            //
            OldValue = *(PULONG)Address;
        }

        Result = Method(Address, SettingBuffer, SettingBufferSize);

        if (Result == NO_ERROR)
        {
            if (OpId == OP_SET || OpId == OP_REMOVE)
            {
                if (SettingsTable[SettingId].SetRoutine)
                {
                    Result = SettingsTable[SettingId].SetRoutine((SETTING_ID)SettingId, SettingsTable[SettingId].Address);
                }

                if (Result == NO_ERROR)
                {
                    if ( TypeVtable[SettingType].WriteSetting )
                    {
                        TypeVtable[SettingType].WriteSetting(SettingsTable[SettingId].Name, SettingsTable[SettingId].Address);
                    }
                }
                else
                {
                    //
                    // Restore old value if user passed invalid parameter
                    //
                    if (SettingType == TYPE_DWORD)
                    {
                        *(PULONG)Address = OldValue;
                    }
                }
            }
        }
    }
    else
    {
        Result = ERROR_INVALID_PARAMETER;
    }

    UnlockSettings( &DfrgSettings.SyncObject );

    return Result;

} // DfrgApiSettingOp


VOID
DfrgApiReadSettings(VOID)
{
    ULONG Index;

    //OutMessage( L"   DfrgApiReadSettings" );

    for (Index = 0; Index < SettingMaxId; Index++)
    {
        int             Result = NO_ERROR;
        NTSTATUS        Status = STATUS_INVALID_PARAMETER;
        SETTING_TYPE    SettingType = SettingsTable[Index].Type;

//#ifdef _DEBUG
//        WCHAR       Mess[MAX_PATH];
//        swprintf( Mess, L"      ID %d Name %ls", SettingsTable[Index].SettingId, SettingsTable[Index].Name );
//        OutMessage( Mess );
//#endif

        if ( TypeVtable[SettingType].ReadSetting )
        {
            Status = TypeVtable[SettingType].ReadSetting(SettingsTable[Index].Name, SettingsTable[Index].Address);
        }

        if ( TYPE_STRING == SettingType && 
            IsEmptyUnicodeString((PCUNICODE_STRING)SettingsTable[Index].Address) &&
            SettingsTable[Index].DefaultValue )
        {
            //
            // This is string setting that should have reasonable value
            //
            Status = STATUS_INVALID_PARAMETER;
        }

        if ( NT_SUCCESS(Status) )
        {
            if ( SettingsTable[Index].SetRoutine )
            {
                Result = SettingsTable[Index].SetRoutine((SETTING_ID)Index, SettingsTable[Index].Address);
            }
        }
        
        if ( !NT_SUCCESS(Status) || Result != NO_ERROR )
        {
            //
            // We were unable to read setting from Registry, or it is invalid
            //
            if ( SettingsTable[Index].DefaultValue )
            {
                //
                // Use the default value if any
                //
                if ( TYPE_DWORD == SettingType )
                {
                    //
                    // Special handling for DWORD to specify constant right in the table without
                    // allocating additional buffer
                    //
                    *(PULONG)SettingsTable[Index].Address = PtrToUlong(SettingsTable[Index].DefaultValue);
                    if (SettingsTable[Index].SetRoutine)
                    {
                        Result = SettingsTable[Index].SetRoutine((SETTING_ID)Index, SettingsTable[Index].Address);
                    }
                }
                else
                {
                    //
                    // We can pass NULL as pointer to buffer size, because size is
                    // not required for set operation
                    //
                    DfrgApiSettingOp(Index, OP_SET, SettingsTable[Index].DefaultValue, NULL);
                }
            } // if ( DefaultValue
        } // if ( !NT_SUCCESS
    } // for

} // end of DfrgApiReadSettings

//
// Initializes settings subsystem
// Registry path is a path where you want setting must be stored
//
void
DfrgInitializeSettings(
    WCHAR   *RegistryPath )
{
    UNREFERENCED_PARAMETER( RegistryPath );


    //UniAllocateAtLeast( &SettingsRegistryPath, wcslen( RegistryPath ) + wcslen( PARAMETERS_SUBKEY ) + 10 );
    //UniInitUnicodeString( &SettingsRegistryPath, RegistryPath );
    //UniAppend( &SettingsRegistryPath, L"\\" );
    //UniAppend( &SettingsRegistryPath, PARAMETERS_SUBKEY );

    UniInitUnicodeString( &SettingsRegistryPath, SERVICES_SUBKEY SERVICE_NAME PARAMETERS_SUBKEY );

    UniInitUnicodeString( &JobsRegistryPath, SERVICES_SUBKEY SERVICE_NAME JOBS_SUBKEY );

#ifdef _DEBUG
    OutMessage( L"Dfrg Initialize Settings. RegistryPath:" );
    OutMessage( SettingsRegistryPath.Buffer );
#endif

    MlsCreate(&DfrgSettings.ExcludedFiles );

    InitializeCriticalSection( &DfrgSettings.SyncObject );

    SettingsInitialized = TRUE;

    DfrgApiReadSettings();

} // end of DfrgInitializeSettings

//
// Uninitializes settings subsystem
//
void
DfrgUninitializeSettings(VOID)
{

#ifdef _DEBUG
    OutMessage( L"Dfrg Uninitialize Settings" );
#endif

    MlsDelete(&DfrgSettings.ExcludedFiles );

    UniFreeUnicodeString( &DfrgSettings.LogFilePath );

    UniFreeUnicodeString( &SettingsRegistryPath );

    UniFreeUnicodeString( &JobsRegistryPath );

    if ( SettingsInitialized )
    {
        DeleteCriticalSection( &DfrgSettings.SyncObject );
    }

    SettingsInitialized = FALSE;

} // end of DfrgUninitializeSettings

