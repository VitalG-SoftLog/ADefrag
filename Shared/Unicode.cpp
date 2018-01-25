/*
    Defrag Engine

    Module name:

        Unicode.cpp

    Abstract:

        Defrag Unicode module. 
        Defines UNICODE_STRING utilities.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/Unicode.cpp,v 1.1 2009/11/24 14:51:31 dimas Exp $
    $Log: Unicode.cpp,v $
    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#include "stdafx.h"

/*
    We have the following conventions for string types:

    1. Each and every UNICODE_STRING MUST have zero-terminated buffer. 
    Length field does not include the zero-terminator. This allows to pass
    our strings directly to system functions.

    2. In all functions string length as input or output parameter (result) 
    is specified in characters not bytes.
*/


// -----------------------------------
//
//      CHAR/WCHAR convertion functions
//
// -----------------------------------

//
// Convert (copy) char string to wchar string 
// DestLength must be specified in characters
//
VOID
UniCopyConvert(
    OUT PWCHAR Dest,
    IN  SIZE_T DestLength, 
    IN  PCHAR  Src)
{
    ANSI_STRING as;
    UNICODE_STRING us;

    RtlInitAnsiString(&as, Src);

    us.Buffer           = Dest;
    us.MaximumLength    = (USHORT)(DestLength*sizeof(WCHAR));
    us.Length           = 0;
    RtlAnsiStringToUnicodeString(&us, &as, FALSE);

    if ( us.Length && (us.Length == us.MaximumLength) )
    {
        us.Length -= sizeof(WCHAR);
    }

    Dest[us.Length/sizeof(WCHAR)] = L'\0';

}

//
// Convert (copy) wchar string to char string 
// DestLength must be specified in characters
//
VOID
UniCopyConvert(
    OUT PCHAR Dest,
    IN  SIZE_T DestLength, 
    IN  PWCHAR Src)
{
    ANSI_STRING as;
    UNICODE_STRING us;

    RtlInitUnicodeString(&us, Src);

    as.Buffer           = Dest;
    as.MaximumLength    = (USHORT)DestLength;

    RtlUnicodeStringToAnsiString(&as, &us, FALSE);

    Dest[as.Length] = '\0';
}

//
// Compare wchar string with char string 
//
LONG
UniCompare(
    IN PWCHAR String1,
    IN PCHAR  String2)
{
    LONG Result;
    UNICODE_STRING UString1;
    UNICODE_STRING UString2;
    ANSI_STRING as;

    RtlInitAnsiString(&as, String2);

    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UString2, &as, TRUE)))
    {
        return 1;
    }

    UString1.Buffer = String1;
    UString1.Length = UString1.MaximumLength = (USHORT)wcslen(String1)*sizeof(WCHAR);

    Result = RtlCompareUnicodeString(&UString1, &UString2, TRUE);

    RtlFreeUnicodeString(&UString2);

    return Result;
}


// -----------------------------------
//
//      UNICODE_STRING functions
//
// -----------------------------------

//
// Check if unicode string should be used.
//
BOOLEAN
IsEmptyUnicodeString(
    PCUNICODE_STRING     String )
{

    if ( !String || 
        !String->MaximumLength || !String->Length || 
        !String->Buffer || !(*(String->Buffer)) )
    {
        return TRUE;
    }

    return FALSE;

}

//
// Make sure unicode string has at least Length characters
//
BOOLEAN
UniAllocateAtLeast(
    IN OUT PUNICODE_STRING String,
    IN     SIZE_T          Length)
{

    if ( Length >= UNICODE_STRING_MAX_CHARS )
    {
        return FALSE;
    }

    size_t  len = Length*sizeof(WCHAR);

    if ( len > String->MaximumLength )
    {
        PWCHAR Temp = (PWCHAR)malloc( len );
        if (!Temp)
        {
            return FALSE;
        }

        if ( String->Buffer )
        {
            memcpy(Temp, String->Buffer, String->Length);
            Temp[String->Length / sizeof(WCHAR)] = L'\0';
            free(String->Buffer);
        }
        else
        {
            Temp[0] = L'\0';
        }

        String->Buffer = Temp;
        String->MaximumLength = (USHORT)len;

    }

    return TRUE;
}


//
// Allocate buffer and init unicode string from char string.
//
BOOLEAN
UniInitUnicodeString(
    IN OUT  PUNICODE_STRING Destination,
    IN      PCHAR           Source)
{
    size_t  len = strlen(Source) + 1;
    if ( !UniAllocateAtLeast(Destination, len) )
    {
        return FALSE;
    }

    UniCopyConvert( Destination->Buffer, len, Source );
    Destination->Length = (USHORT)(len-1)*sizeof(WCHAR);

    return TRUE;
}

//
// Allocate buffer and init unicode string from wchar string.
//
BOOLEAN
UniInitUnicodeString(
    IN OUT  PUNICODE_STRING Destination,
    IN      PWCHAR          Source)
{
    size_t  len = wcslen(Source) + 1;
    if ( !UniAllocateAtLeast(Destination, len) )
    {
        return FALSE;
    }

    wcscpy(Destination->Buffer, Source);
    Destination->Length = (USHORT)(len-1)*sizeof(WCHAR);

    return TRUE;
}

//
// Free buffer and set pointer to NULL.
//
VOID
UniFreeUnicodeString(
    IN  PUNICODE_STRING String )
{

    if ( String )
    {
        if ( String->Buffer )
        {
            //free( String->Buffer );
            free( String->Buffer );
        }
        String->Buffer          = NULL;
        String->Length          = 0;
        String->MaximumLength   = 0;
    }

}



//
// Copy one unicode string to another unicode string. 
// If necessary the destination string is reallocated 
// to hold entire source string.
// The Destination unicode string is NULL terminated.
//
BOOLEAN
UniCopy(
    OUT PUNICODE_STRING     Destination,
    IN  PCUNICODE_STRING    Source)
{
    if (!UniAllocateAtLeast(Destination, UniGetLen(Source) + 1 ))
    {
        return FALSE;
    }

    RtlCopyUnicodeString(Destination, Source);
    Destination->Buffer[UniGetLen(Source)] = L'\0';

    return TRUE;
}

//
// Append one unicode string to another unicode string. 
// If necessary the destination string is reallocated 
// to hold entire string.
// The Destination unicode string is NULL terminated.
//
BOOLEAN
UniAppend(
    IN OUT  PUNICODE_STRING     Destination,
    IN      PCUNICODE_STRING    Source )
{
    size_t  len = UniGetLen(Destination) + UniGetLen(Source);
    if ( !UniAllocateAtLeast(Destination, len + 1) )
    {
        return FALSE;
    }

    memcpy( Destination->Buffer+UniGetLen(Destination), Source->Buffer, Source->Length );
    Destination->Buffer[len] = L'\0';
    Destination->Length = (USHORT)len*sizeof(WCHAR);

    return TRUE;
}

//
// Append a NULL terminated Source string of unicode chars to unicode string. 
// If necessary the destination string is reallocated 
// to hold entire string.
// The Destination unicode string is NULL terminated.
//
BOOLEAN
UniAppend(
    IN OUT  PUNICODE_STRING     Destination,
    IN      PWCHAR              Source )
{
    size_t  srclen = wcslen(Source);
    size_t  len = UniGetLen(Destination) + srclen;

    if ( !UniAllocateAtLeast(Destination, len + 1) )
    {
        return FALSE;
    }

    memcpy( Destination->Buffer+UniGetLen(Destination), Source, srclen*sizeof(WCHAR) );
    Destination->Buffer[len] = L'\0';
    Destination->Length = (USHORT)len*sizeof(WCHAR);

    return TRUE;
}


// -----------------------------------
//
//      MULTI_SZ list functions
//
// -----------------------------------

//
// Initialize MULTI_SZ list
//
BOOLEAN
MlsCreate(
    OUT PUNICODE_STRING List)
{
    List->Length = sizeof(WCHAR);
    List->MaximumLength = MLS_DEFAULT_SIZE;

    List->Buffer = (PWCHAR) malloc( List->MaximumLength );

    if (!List->Buffer)
    {
        return FALSE;
    }

    *(List->Buffer) = L'\0';

    return TRUE;
}

//
// Free MULTI_SZ list
//
VOID
MlsDelete(
    IN PUNICODE_STRING List)
{
    if ( List )
    {
        if ( List->Buffer )
        {
            free(List->Buffer);
        }
        List->Buffer        = NULL;
        List->Length        = 0;
        List->MaximumLength = 0;
    }
}

//
// Get list length and validate access to its memory
//
SIZE_T
MlsGetListRealLength(
    IN PWCHAR List)
{
    SIZE_T Length = 1, ItemLength;
    while (*List)
    {
        ItemLength = wcslen(List) + 1;
        Length  += ItemLength;
        List    += ItemLength;
    }

    return Length;
}

//
// Returns pointer to item if it is present in the MULTI_SZ-list
//
// TODO: specify comparison function for items, for example
// user->native->user translation for paths. Currently we use wcsicmp.
//
PWCHAR
MlsIsItemPresent(
    IN  PWCHAR  List,
    IN  PWCHAR  Item,
    OUT PSIZE_T FoundItemLength)
{
    SIZE_T CurrentItemLength;

    while (*List)
    {
        CurrentItemLength = wcslen(List);

        if (!_wcsicmp(List, Item))
        {
            if (FoundItemLength)
            {
                *FoundItemLength = CurrentItemLength;
            }

            return List;
        }

        List += CurrentItemLength + 1;
    }

    return NULL;
}

//
// Add PWCHAR string buffer to the end of MULTI_SZ list
//
BOOLEAN
MlsAppend(
    IN OUT PUNICODE_STRING List,
    IN     PWCHAR          Item)
{
    USHORT ItemLength = (USHORT)wcslen(Item);

    //ASSERT(UniGetLen(List) && UniGetLen(List) != 2);

    if (!UniAllocateAtLeast(List, UniGetLen(List) + ItemLength + 1))
    {
        return FALSE;
    }

    memcpy(List->Buffer + UniGetLen(List) - 1, Item, (ItemLength + 1)*sizeof(WCHAR));
    *(List->Buffer + UniGetLen(List) + ItemLength) = L'\0';
    List->Length += (ItemLength + 1)*sizeof(WCHAR);

    return TRUE;
}

//
// Allocates kernel memory and safely copies user-mode list to it.
// Caller must free allocated memory.
//
PWCHAR
MlsDuplicateList(
    IN PWCHAR List)
{
    PWCHAR TempBuffer = NULL;

    __try
    {
        //
        // This call can throw an exception
        //
        SIZE_T Size = MlsGetListRealLength(List)*sizeof(WCHAR);

        if (!Size)
        {
            __leave;
        }

        //
        // Create copy of user-mode buffer to protect against unexpected exceptions
        //
        TempBuffer = (PWCHAR) malloc( Size );

        if (!TempBuffer)
        {
            __leave;
        }

        memcpy(TempBuffer, List, Size);

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        if (TempBuffer)
        {
            free(TempBuffer);
            TempBuffer = NULL;
        }

    }

    return TempBuffer;
}

