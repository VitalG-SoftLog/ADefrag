// =================================================
//
// Unicode.h : Defines UNICODE_STRING utilities.
//
// =================================================

#ifndef __UNISTR_H__
#define __UNISTR_H__



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
//      CHAR/WCHAR Convertion functions
//
// -----------------------------------

//
// Convert (copy) wchar string to char string
// DestLength must be specified in characters
//
VOID
UniCopyConvert(
    OUT PWCHAR Dest,
    IN  SIZE_T DestLength,
    IN  PCHAR  Src);

//
// Convert (copy) char string to wchar string 
// DestLength must be specified in characters
//
VOID
UniCopyConvert(
    OUT PCHAR Dest,
    IN  SIZE_T DestLength,
    IN  PWCHAR Src);

//
// Compare wchar string with char string 
//
LONG
UniCompare(
    IN PWCHAR String1,
    IN PCHAR  String2);


// -----------------------------------
//
//      UNICODE_STRING functions
//
// -----------------------------------

//
// Get length of UNICODE_STRING in characters
//
FORCEINLINE
SIZE_T
UniGetLen(
    IN PCUNICODE_STRING String)
{
    return String->Length/sizeof(WCHAR);
}

//
// Check if unicode string should be used.
//
BOOLEAN
IsEmptyUnicodeString(
    PCUNICODE_STRING     String );

//
// Make sure unicode string has at least Length characters
//
BOOLEAN
UniAllocateAtLeast(
    IN OUT PUNICODE_STRING String,
    IN     SIZE_T          Length);

//
// Allocate buffer and init unicode string from char string.
//
BOOLEAN
UniInitUnicodeString(
    IN OUT  PUNICODE_STRING Destination,
    IN      PCHAR           Source);

//
// Allocate buffer and init unicode string from wchar string.
//
BOOLEAN
UniInitUnicodeString(
    IN OUT  PUNICODE_STRING Destination,
    IN      PWCHAR          Source);

//
// Free buffer and set pointer to NULL.
//
VOID
UniFreeUnicodeString(
    IN  PUNICODE_STRING String );

//
// Copy one unicode string to another unicode string. 
// If necessary the destination string is reallocated 
// to hold entire source string.
//
BOOLEAN
UniCopy(
    OUT PUNICODE_STRING     Destination,
    IN  PCUNICODE_STRING    Source);

//
// Append one unicode string to another unicode string. 
// If necessary the destination string is reallocated 
// to hold entire string.
// The Destination unicode string is NULL terminated.
//
BOOLEAN
UniAppend(
    OUT PUNICODE_STRING     Destination,
    IN  PCUNICODE_STRING    Source );

//
// Append a NULL terminated Source string of unicode chars to unicode string. 
// If necessary the destination string is reallocated 
// to hold entire string.
// The Destination unicode string is NULL terminated.
//
BOOLEAN
UniAppend(
    IN OUT  PUNICODE_STRING     Destination,
    IN      PWCHAR              Source );


// -----------------------------------
//
//      MULTI_SZ list functions
//
// -----------------------------------

#define MLS_DEFAULT_SIZE    16

//
// Initialize MULTI_SZ list
//
BOOLEAN
MlsCreate(
    OUT PUNICODE_STRING List);

//
// Free MULTI_SZ list
//
VOID
MlsDelete(
    IN PUNICODE_STRING List);

//
// Get list length and validate access to its memory
//
SIZE_T
MlsGetListRealLength(
    IN PWCHAR List);

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
    OUT PSIZE_T FoundItemLength);

//
// Add PWCHAR string buffer to the end of MULTI_SZ list
//
BOOLEAN
MlsAppend(
    IN OUT PUNICODE_STRING List,
    IN     PWCHAR          Item);

//
// Allocates kernel memory and safely copies user-mode list to it.
// Caller must free allocated memory.
//
PWCHAR
MlsDuplicateList(
    IN PWCHAR List);

#endif // !__UNISTR_H__