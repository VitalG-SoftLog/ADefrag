/*
    NTFS utilities module

    Module name:

        NtfsUtils.cpp

    Abstract:

        NTFS utilities module. 
        Defines functions used for NTFS disk analysis 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/NtfsUtils.cpp,v 1.6 2009/12/24 15:02:51 dimas Exp $
    $Log: NtfsUtils.cpp,v $
    Revision 1.6  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.5  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.4  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.3  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"


WCHAR       *NTFS_STREAM_ATTRIBUTE_NAME[] = 
{
    L"",
    L"$STANDARD_INFORMATION",
    L"$ATTRIBUTE_LIST",
    L"$FILE_NAME",
    L"$OBJECT_ID",
    L"$SECURITY_DESCRIPTOR",
    L"$VOLUME_NAME",
    L"$VOLUME_INFORMATION",
    L"$DATA",
    L"$INDEX_ROOT",
    L"$INDEX_ALLOCATION",
    L"$BITMAP",
    L"$REPARSE_POINT",
    L"$EA_INFORMATION",
    L"$EA",
    L"$PROPERTY_SET",
    L"$LOGGED_UTILITY_STREAM"
};

PWCHAR
GetStreamTypeName( 
    NTFS_STREAM_ATTRIBUTE_TYPE  MftStreamType )
{

    ULONG   StreamType;

    if ( MftStreamType > MftAttrLoggedUtilityStream )
    {
        StreamType = 0;
    }
    else
    {
        StreamType = MftStreamType >> 4;
    }

    return NTFS_STREAM_ATTRIBUTE_NAME[StreamType];

}



//
// Check if MFT record is valid and restore two last bytes in every sector from UsaArray.
//
int
NtfsCookMftRecord( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PMFT_FILE_RECORD_HEADER MftRecord )
{
    int                     Result = DFRG_ERROR_SKIP; //ERROR_INVALID_PARAMETER;
    MFT_RECORD_HEADER       MftHeader = MftRecord->MftHeader;


    __try
    {
        //
        // Check signature
        //
        if ( MftHeader.Type != 'ELIF' || 
            MftHeader.UsaOffset >= VolumeCtx->NtfsData.BytesPerFileRecordSegment ) 
        {
            return  Result;
        }

        //
        // Check index (Inode)
        // TOD: check in XP only ?
        //
        //if ( MFTRecordIndexber != MftIndex )
        //{
        //    return  Result;
        //}

        PWORD   UsaArray = (PWORD)((PBYTE)MftRecord + MftHeader.UsaOffset);
        PWORD   UsaFixPtr;
        DWORD   UsaFixOffset;

        for ( int i=1; i<(int)MftHeader.UsaCount; i++ ) 
        {
            UsaFixOffset = i*VolumeCtx->NtfsData.BytesPerSector - sizeof(WORD);
            UsaFixPtr = (PWORD)((PBYTE)MftRecord + UsaFixOffset);

            if ( UsaFixOffset >= VolumeCtx->NtfsData.BytesPerFileRecordSegment ||
                *UsaFixPtr != *UsaArray )
            {
                Result = DFRG_ERROR_FS_DATA_CORRUPTED;
                return Result;
            }
            *UsaFixPtr = *(UsaArray+i);
        }

        Result = NO_ERROR;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"NtfsCookMftRecord(). Exception !!!", Result, ERR_OS );
    }

    return Result;

} // end of NtfsCookMftRecord

//
// Calculates file's cluster map from Mft data runs.
// Each data run has next format:
//
int 
NtfsGetClustersMapFromMft( 
    PMFT_NONRESIDENT_ATTRIBUTE  Attribute, 
    PDEFRAG_STREAM_CONTEXT      StreamCtx,
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx )
{
    int                     Result = NO_ERROR;

    PBYTE                   RunData;
    int                     RunDataLength, RunLengthSize, RunOffsetSize;
    RUN_EXTENT_ITEM         ExtentLcnDiff = { 0 }, ExtentLength = { 0 };
    ULONG                   ExtentNum = 0;
    ULONG                   MaxExtentNum;
    LARGE_INTEGER           Vcn = { 0 };
    LARGE_INTEGER           Lcn = { 0 };

#ifdef _DEBUG
            WCHAR       Mess[MAX_PATH];
#endif

__try
{
    RunData = (PBYTE)Attribute + Attribute->RunArrayOffset;
    RunDataLength = Attribute->Attribute.Length - Attribute->RunArrayOffset;

    //
    //MaxExtentNum = BUFFER_ITEMS_NUM;
    //PRETRIEVAL_POINTERS_BUFFER  ClustersMap = (PRETRIEVAL_POINTERS_BUFFER)VolumeCtx->ClustersMapBuffer;
    //
    // TODO:
    // Allocate exect amount of memory. Now Should be enough
    //
    MaxExtentNum = RunDataLength/2 + 2;
    size_t NewLength = sizeof(LARGE_INTEGER)*2*(MaxExtentNum-1);

    size_t OldLength = 0;

    PRETRIEVAL_POINTERS_BUFFER  ClustersMap = NULL;

#ifdef _DEBUGW
    if ( !wcscmp( StreamCtx->FileCtx->FName, L"A0001555.cmd" ) || StreamCtx->FileCtx->FId.FileId == 0 ||
         !wcscmp( StreamCtx->FileCtx->FName, L"I386" ) || StreamCtx->FileCtx->FId.FileId == 0x000100000000019d )
    {
        Result = NO_ERROR;
    }
#endif

    if ( StreamCtx->ClustersMap )
    {
        //
        // This is continuation of existing stream.
        // Restore last VCN/LCN values.
        //
        int k = StreamCtx->ClustersMap->ExtentCount-1;
        Vcn = StreamCtx->ClustersMap->Extents[k].NextVcn;
#ifdef _DEBUG
        if ( Vcn.QuadPart != Attribute->StartingVcn )
        {
            swprintf( Mess, L"      Continuation of Attr (list) not match: %I64d != %I64d", 
                Vcn.QuadPart, Attribute->StartingVcn );
            OutMessage( Mess );
            GetInput();
            __leave;
        }
#endif
#if 0
        while ( k >= 0 )
        {
            if ( VIRTUAL_LCN != ClustersMap->Extents[k].Lcn.QuadPart )
            {
                Lcn = ClustersMap->Extents[k].Lcn;
                break;
            }
            k--;
        }
#endif

        OldLength = sizeof(LARGE_INTEGER)*2*(StreamCtx->ClustersMap->ExtentCount-1);

        ClustersMap = (PRETRIEVAL_POINTERS_BUFFER) 
            malloc( sizeof(RETRIEVAL_POINTERS_BUFFER) + OldLength + NewLength );
        
        if ( !ClustersMap )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        memcpy( ClustersMap, StreamCtx->ClustersMap, sizeof(RETRIEVAL_POINTERS_BUFFER) + OldLength );

        free( StreamCtx->ClustersMap );

        StreamCtx->ClustersMap = ClustersMap;

        ExtentNum = ClustersMap->ExtentCount;
        MaxExtentNum += ExtentNum;

    }
    else
    {
        ClustersMap = (PRETRIEVAL_POINTERS_BUFFER) 
            malloc( sizeof(RETRIEVAL_POINTERS_BUFFER) + NewLength );
        
        if ( !ClustersMap )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        ClustersMap->ExtentCount = 0;
        ClustersMap->StartingVcn.QuadPart = 0; 

        StreamCtx->ClustersMap = ClustersMap;
    }



    for( int i=0; RunData[i] && i<RunDataLength; )
    {
        //
        // TODO: realloc
        //
        if ( ExtentNum == MaxExtentNum ) // BUFFER_ITEMS_NUM 
        {
#ifdef _DEBUG
            OutMessage( L"NtfsGetClustersMapFromMft: Map is out of memory" );
            *Mess = L'\0';
            for( i=0; i<RunDataLength; i++ )
            {
                swprintf( Mess+wcslen(Mess), L"%02X ", (int)RunData[i] );
                if ( !((i+1)%16) || (i+1) == RunDataLength )
                {
                    OutMessage( Mess );
                    Mess[0] = 0;
                }
            }
#endif
            Result = DFRG_ERROR_FS_DATA_CORRUPTED; // ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        RunLengthSize = RunData[i] & 0x0F;
        RunOffsetSize = (RunData[i] & 0xF0) >> 4;

        if ( i+RunLengthSize+RunOffsetSize >= RunDataLength || 
            RunLengthSize  > 8 || RunOffsetSize > 8 )
        {
            Result = DFRG_ERROR_FS_DATA_CORRUPTED;
            __leave;
        }

        ExtentLcnDiff.QuadPart = 0;
        ExtentLength.QuadPart = 0;

        register int j;

        //
        // Get Length, byte by byte
        //
        i++;
        for( j=0; j<RunLengthSize; j++, i++ )
        {
            ExtentLength.Byte[j] = RunData[i];
        }

        //
        // Get Offset from previous Lcn, byte by byte
        //
        if ( RunOffsetSize )
        {
            for( j=0; j<RunOffsetSize; j++, i++ )
            {
                ExtentLcnDiff.Byte[j] = RunData[i];
            }

            if ( ExtentLcnDiff.Byte[j-1] & 0x80 )
            {
                //
                // Negative offset
                //
                for( ; j<8; j++ )
                {
                    ExtentLcnDiff.Byte[j] = 0xFF;
                }
            }
        }

        Vcn.QuadPart += ExtentLength.QuadPart;
        Lcn.QuadPart += ExtentLcnDiff.QuadPart;

        ClustersMap->Extents[ExtentNum].NextVcn = Vcn;

        //if ( ExtentLength.QuadPart )
        if ( ExtentLcnDiff.QuadPart )
        {
            ClustersMap->Extents[ExtentNum].Lcn = Lcn;
            //
            // Update stream length/file clusters num. 
            // Take into account only real clusters.
            // TODO: optimize and/or move from here
            //
            StreamCtx->DataSize.QuadPart += ExtentLength.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
            StreamCtx->FileCtx->TotalClustersNum.QuadPart += ExtentLength.QuadPart;
            StreamCtx->FileCtx->TotalFragmentsNum.QuadPart++;
        }
        else
        {
            ClustersMap->Extents[ExtentNum].Lcn.QuadPart = VIRTUAL_LCN;
        }

        ExtentNum++;
        ClustersMap->ExtentCount = ExtentNum;


    } // for( i

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"NtfsGetClustersMapFromMft(). Exception !!!", Result, ERR_OS );
}

    return Result;

} // end of NtfsGetClustersMapFromMft

//
// Parse Attribute set (Mft record or Attributes list)
//
int
NtfsExploreAttribute( 
    IN  PMFT_ATTRIBUTE          Attribute,
    IN  int                     ListDepth, 
    OUT PDEFRAG_FILE_CONTEXT    FileCtx,
    IN OUT int                  *AttrListsNum )
{
    int                         Result = NO_ERROR; // DFRG_ERROR_SKIP; 

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx;
    PMFT_NONRESIDENT_ATTRIBUTE  NonResident;
    PMFT_RESIDENT_ATTRIBUTE     Resident;

    PMFT_FILENAME_ATTRIBUTE     FileNameAttribute;
    PMFT_STANDARD_INFORMATION   StandardInformation;

    size_t                      Length;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];
#endif


__try
{

    VolumeCtx = FileCtx->VolumeCtx;

    if ( !Attribute->Nonresident ) 
    {
        Resident = (PMFT_RESIDENT_ATTRIBUTE)Attribute;

        switch ( Attribute->AttributeType )
        {
        case MftAttrStandardInformation:
            //
            // Standard Information & Name should be always resident
            //
            StandardInformation = (PMFT_STANDARD_INFORMATION)((PBYTE)Resident + Resident->ValueOffset);

            FileCtx->CreationTime.QuadPart   = StandardInformation->CreationTime;
            FileCtx->ChangeTime.QuadPart     = StandardInformation->LastWriteTime;
            FileCtx->LastAccessTime.QuadPart = StandardInformation->LastAccessTime;
            FileCtx->Attributes              = StandardInformation->FileAttributes;

            break;

        case MftAttrFileName:
            //
            // Standard Information & Name should be always resident
            //
            FileNameAttribute = (PMFT_FILENAME_ATTRIBUTE)((PBYTE)Resident + Resident->ValueOffset);

            if ( FileCtx->FId.FileId == FileNameAttribute->ParentDirectory.FileId )
            {
                FileCtx->DefragFlags |= (DEFRAG_FILE_DIRECTORY | DEFRAG_FILE_ROOT_DIR);
            }
            else
            {
                FileCtx->ParentDirId.FileId = FileNameAttribute->ParentDirectory.FileId;
            }

            Length = FileNameAttribute->NameLength;
            if ( Length )
            {
                //if ( NO_NAME == FileCtx->NameType ||
                //    LONG_NAME != FileCtx->NameType &&
                //    LONG_NAME == FileNameAttribute->NameType )
                if ( !(FileCtx->DefragFlags2 & (DEFRAG_FILE2_SHORT_NAME | DEFRAG_FILE2_LONG_NAME)) ||
                    !(FileCtx->DefragFlags2 & DEFRAG_FILE2_LONG_NAME) && LONG_NAME == FileNameAttribute->NameType )
                {
                    //
                    // Allocate and save name
                    //
                    if ( FileCtx->FName )
                    {
                        free( FileCtx->FName );
                        FileCtx->FName = NULL;
                    }

                    FileCtx->FName = (PWCHAR)malloc( (Length+1)*sizeof(WCHAR) );
                    if ( !FileCtx->FName )
                    {
                        Result = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }

                    wcsncpy( FileCtx->FName, FileNameAttribute->Name, Length );
                    FileCtx->FName[Length] = L'\0';

                    //FileCtx->NameType = FileNameAttribute->NameType;
                    if ( LONG_NAME == FileNameAttribute->NameType )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_LONG_NAME;
                        FileCtx->DefragFlags2 &= ~(DEFRAG_FILE2_SHORT_NAME);
                    }
                    else
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_SHORT_NAME;
                        FileCtx->DefragFlags2 &= ~(DEFRAG_FILE2_LONG_NAME);
                    }
                }
            }
            break;

        case MftAttrData:

            //
            // Resident data stream.
            // We do not interesting in resident data
            //
            break;

        case MftAttrList:
            //
            // Skip now
            //
            (*AttrListsNum)++;
            break;

            break;
        case MftAttrObjectId:
            break;
        case MftAttrSecurityDescriptor:
            break;
        case MftAttrVolumeName:
            break;
        case MftAttrVolumeInformation:
            break;
        case MftAttrIndexRoot:
            break;
        case MftAttrIndexAllocation:
            break;
        case MftAttrBitmap:
            break;
        case MftAttrReparsePoint:
            //OutMessage( L"Reparse point attribute" );
            break;
        case MftAttrEAInformation:
            break;
        case MftAttrEA:
            break;
        case MftAttrPropertySet:
            break;
        case MftAttrLoggedUtilityStream:
            break;

        case MftAttrUninit:
        default:
            //
            // Unknown attribute
            //
            OutMessage( L"Unknown resident attribute" );
            break;
        }
    }
    else // NonResident - $DATA & Other
    {
        NonResident = (PMFT_NONRESIDENT_ATTRIBUTE)Attribute;
/*
//
// this is common block in DfrgMarkupFile
//
        if ( (Attribute->Flags & MFT_ATTR_COMPRESSED) )
        {
            FileCtx->DefragFlags |= DEFRAG_FILE_COMPRESSED;
        }
        if ( (Attribute->Flags & MFT_ATTR_ENCRYPTED) )
        {
            FileCtx->DefragFlags |= DEFRAG_FILE_ENCRYPTED;
        }
        if ( (Attribute->Flags & MFT_ATTR_SPARSED) )
        {
            FileCtx->DefragFlags |= DEFRAG_FILE_SPARSED;
        }
*/
        switch ( Attribute->AttributeType )
        {
        case MftAttrStandardInformation:
        case MftAttrFileName:
            //
            // Standard Information & Name should be always resident
            //
            OutMessage( L"NonResident FileName/StandardInformation" );
            break;

        case MftAttrList:
            //
            // Skip now
            //
            (AttrListsNum)++;
            break;

        case MftAttrData:
        case MftAttrObjectId:
        case MftAttrSecurityDescriptor:
        case MftAttrVolumeName:
        case MftAttrVolumeInformation:
        case MftAttrIndexRoot:
        case MftAttrIndexAllocation:
        case MftAttrBitmap:
        case MftAttrReparsePoint:
        case MftAttrEAInformation:
        case MftAttrEA:
        case MftAttrPropertySet:
        case MftAttrLoggedUtilityStream:
        {
            //if ( MftAttrReparsePoint == Attribute->AttributeType )
            //{
            //    OutMessage( L"Reparse point attribute" );
            //}
            //
            // Stream. We should get and save it's ClustersMap
            //
            PDEFRAG_STREAM_CONTEXT      StreamCtx = NULL;

            //
            // Get stream (attribute) name 
            //
            Length = Attribute->NameLength;
            PWCHAR AttrName = (PWCHAR)((PBYTE)Attribute + Attribute->NameOffset);

            BOOLEAN     IsStreamExist = FALSE;

            if ( ListDepth )
            {
                //
                // Check if this attribute stream is already allocated.
                // TODO: test
                //
                StreamCtx = FileCtx->FirstStream;

                while( StreamCtx )
                {
#ifdef _DEBUGW
                    //
                    // Looks like AttributeId is not valuable
                    //
                    if ( StreamCtx->AttributeId &&
                        StreamCtx->AttributeId == Attribute->AttributeId )
                        //&& MftAttrData != Attribute->AttributeType )
                    {
                        swprintf( Mess, L"          Continuation of Attr Id %u", (DWORD)Attribute->AttributeId );
                    }
#endif
                    if ( StreamCtx->MftStreamType == Attribute->AttributeType &&
                        ( !StreamCtx->StreamName && !Length ||
                        Length == StreamCtx->NameLength && 
                        !wcsncmp(StreamCtx->StreamName, AttrName, Length ) ) )
                    {
#ifdef _DEBUG
                        swprintf( Mess, L"          Continuation of %ls", GetStreamTypeName( StreamCtx->MftStreamType ) );

                        if ( StreamCtx->StreamName )
                        {
                            swprintf( Mess+wcslen(Mess), L": %ls =? ", StreamCtx->StreamName );
                        }
                        if ( Length )
                        {
                            wcsncat( Mess, AttrName, Length );
                        }

                        OutMessage( Mess );
#endif
                        IsStreamExist = TRUE;
                        break;
                    }

                    StreamCtx = StreamCtx->NextStream;
                }
            }

            if ( !StreamCtx ) // new stream
            {
                //
                // Allocate next stream struct
                //
                StreamCtx = (PDEFRAG_STREAM_CONTEXT) malloc( sizeof(DEFRAG_STREAM_CONTEXT) );
                if ( !StreamCtx )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }

                memset( StreamCtx, 0, sizeof(DEFRAG_STREAM_CONTEXT) );

                if ( !FileCtx->FirstStream )
                {
                    FileCtx->FirstStream = StreamCtx;
                }
                else
                {
                    PDEFRAG_STREAM_CONTEXT  LastStreamCtx = FileCtx->FirstStream;
                    while ( LastStreamCtx->NextStream )
                    {
                        LastStreamCtx = LastStreamCtx->NextStream;
                    }
                    LastStreamCtx->NextStream = StreamCtx;
                }

                StreamCtx->FileCtx = FileCtx;
                StreamCtx->MftStreamType = Attribute->AttributeType;
                //
                // TODO: Check AttributeId 
                //
                //StreamCtx->AttributeId = Attribute->AttributeId;
                //FileCtx->StreamNum++;

                if ( Length )
                {
                    StreamCtx->StreamName = (PWCHAR)malloc( (Length+1)*sizeof(WCHAR) );
                    if ( !StreamCtx->StreamName )
                    {
                        Result = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                    wcsncpy( StreamCtx->StreamName, AttrName, Length );
                    StreamCtx->StreamName[Length] = L'\0';
                    StreamCtx->NameLength = (DWORD)Length;
                }

                if ( MftAttrData == Attribute->AttributeType && !Length ||                         // default file unnamed data stream
                     MftAttrIndexAllocation == StreamCtx->MftStreamType && 
                        (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) &&
                        StreamCtx->NameLength == 4 && !wcscmp( StreamCtx->StreamName, L"$I30" ) )  // default directory $I30 stream
                {
                    //
                    // Default $DATA stream. Mainly interesting in it.
                    //
                    if ( !FileCtx->DefaultStream )
                    {
                        FileCtx->DefaultStream = StreamCtx;
                    }
                    else
                    {
#ifdef _DEBUG
                        if ( !ListDepth )
                        {
                            swprintf( Mess, L"          Second default %ls stream ", GetStreamTypeName( StreamCtx->MftStreamType ) );
                            if ( Length )
                            {
                                wcsncat( Mess, AttrName, Length );
                            }
                            OutMessage( Mess );
                        }
#endif
                    }
                }
            } // if  ( new stream

            Result = NtfsGetClustersMapFromMft( NonResident, StreamCtx, VolumeCtx );
            if ( NO_ERROR != Result )
            {   
                StreamCtx->DataSize.QuadPart = 0;
                //__leave;
            }
            else
            {
                //StreamCtx->DataSize.QuadPart *= VolumeCtx->NtfsData.BytesPerCluster;
                //FileCtx->TotalSize.QuadPart += StreamCtx->DataSize.QuadPart;
            }

        }
            break;

        case MftAttrUninit:
        default:
            //
            // Unknown attribute
            //
            OutMessage( L"Unknown non resident attribute" );
            break;
        } // switch

    } // if ( Resident

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"NtfsExploreAttribute(). Exception !!!", Result, ERR_OS );
}
    return Result;

} // end of NtfsExploreAttribute


//
// Parse Attributes set (Mft record or Attributes list)
//
int
NtfsParseAttributesList( 
    IN  PBYTE                   AttributeBuffer,
    IN  DWORD                   BufferLength,
    IN  int                     ListDepth,
    OUT PDEFRAG_FILE_CONTEXT    FileCtx )
{
    int                         Result = NO_ERROR; // DFRG_ERROR_SKIP;

    DWORD                       AttributeOffset = 0;
    PMFT_ATTRIBUTE_LIST         Attribute = NULL;

    PBYTE                       MftRecordsBuffer = NULL;
    PMFT_FILE_RECORD_HEADER     MftRecord;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];

    OutMessage( L"" );
    OutMessage( L"    NtfsParseAttributesList()." );
    //PrintClustersMap( ListStreamCtx->ClustersMap );
#endif


    __try
    {
        PDEFRAG_VOLUME_CONTEXT      VolumeCtx = FileCtx->VolumeCtx;
        DWORD                       RecordPerClusters = 
            (DWORD)(VolumeCtx->NtfsData.BytesPerCluster / VolumeCtx->NtfsData.BytesPerFileRecordSegment);
        DWORD                       BufferLength = 
            max( VolumeCtx->NtfsData.BytesPerCluster, VolumeCtx->NtfsData.BytesPerFileRecordSegment );


        MftRecordsBuffer = (PBYTE) malloc( BufferLength );
        if ( !MftRecordsBuffer )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        MFT_INODE_REFERENCE     FileMftIndex;
        MFT_INODE_REFERENCE     AttrMftIndex;
        MFT_INODE_REFERENCE     PrevAttrMftIndex = { 0 };

        FileMftIndex.FileId = FileCtx->FId.FileId;
        FileMftIndex.SequenceNumber = 0;

        LARGE_INTEGER           DiskOffset;
        DWORD                   BufferOffset;
        DWORD                   BytesRead;

        //
        // Enumerate attributes in list.
        //
        for ( ; AttributeOffset < BufferLength; AttributeOffset += Attribute->Length ) 
        {
            Attribute = (PMFT_ATTRIBUTE_LIST)(AttributeBuffer + AttributeOffset);

            if ( AttributeOffset + 4 > BufferLength ||
                Attribute->Length < 3 ||
                AttributeOffset + Attribute->Length > BufferLength ||
                MftAttrRecordEnd == Attribute->AttributeType ) // Last attribute. It's not a case for attributes list but who know.
            {
                //
                // End of record. All this cases are valid for attributes list.
                //
                Result = NO_ERROR;
                break;
            }

            AttrMftIndex.FileId = Attribute->AttributeInodeReference.FileId;
            AttrMftIndex.SequenceNumber = 0;

            //
            // Skip too high nodes
            //
            if ( AttrMftIndex.FileId >= (ULONGLONG)VolumeCtx->ValidRecordsNum.QuadPart )
            {
                //
                // TODO: What is this MftIndexHighPart == 0x18, etc.
                //
#ifdef _DEBUG
                swprintf( Mess, L"      AttrRec: %016I64X >= MaxRec: %016I64X   Skip", 
                    Attribute->AttributeInodeReference.FileId,
                    VolumeCtx->ValidRecordsNum.QuadPart );
                OutMessage( Mess );
#endif
                continue;
            }

            //
            // Skip self referencing
            //
            if ( AttrMftIndex.FileId == FileMftIndex.FileId )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      FileRec: %016I64X = AttrRec: %016I64X   Skip", 
                    FileCtx->FId.FileId,
                    Attribute->AttributeInodeReference.FileId );
                OutMessage( Mess );
#endif
                continue;
            }

            //
            // Skip reserved nodes
            //
            if ( AttrMftIndex.FileId <= 23 )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      AttrRec: %016I64X <=  Reserved node ???    23   Skip", 
                    Attribute->AttributeInodeReference.FileId,
                    VolumeCtx->ValidRecordsNum.QuadPart );
                OutMessage( Mess );
#endif
                continue;
            }

            //
            // Skip already processed record
            //
            if ( PrevAttrMftIndex.FileId == AttrMftIndex.FileId )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      PrevRec: %016I64X = AttrRec: %016I64X   Skip", 
                    PrevAttrMftIndex.FileId,
                    Attribute->AttributeInodeReference.FileId );
                OutMessage( Mess );
#endif
                continue;
            }

#ifdef _DEBUG
            swprintf( Mess, L"      FileRec: %016I64X   AttrRec: %016I64X   Attr: %ls ", 
                FileCtx->FId.FileId,
                Attribute->AttributeInodeReference.FileId,
                GetStreamTypeName( Attribute->AttributeType ) );
            if ( Attribute->NameLength )
            {
                wcsncat( Mess, (PWCHAR)((PBYTE)Attribute+Attribute->NameOffset), Attribute->NameLength );
            }
            OutMessage( Mess );
#endif

            if ( RecordPerClusters  )
            {
                DiskOffset.QuadPart = 
                    (AttrMftIndex.FileId / RecordPerClusters) * VolumeCtx->NtfsData.BytesPerCluster;
                BufferOffset = 
                    (AttrMftIndex.FileId % RecordPerClusters) * VolumeCtx->NtfsData.BytesPerFileRecordSegment;
            }
            else
            {
                DiskOffset.QuadPart = 
                    AttrMftIndex.FileId * VolumeCtx->NtfsData.ClustersPerFileRecordSegment * VolumeCtx->NtfsData.BytesPerCluster;
                BufferOffset = 0;
            }

            Result = DfrgReadStreamRaw( 
                        VolumeCtx->MftFileCtx->DefaultStream, 
                        &DiskOffset, 
                        (PBYTE)MftRecordsBuffer, 
                        BufferLength,
                        TRUE,
                        (PULONG)&BytesRead );
            if ( NO_ERROR != Result )
            {
                continue;
            }

            MftRecord = (PMFT_FILE_RECORD_HEADER)(MftRecordsBuffer + BufferOffset);

#ifdef _DEBUG
            swprintf( Mess, L" Read AttrRec: %016I64X   BaseRec: %016I64X", 
                Attribute->AttributeInodeReference.FileId, 
                MftRecord->BaseFileRecord.FileId );
            OutMessage( Mess );
#endif
            
            Result = NtfsCookMftRecord( VolumeCtx, MftRecord ); 
            if ( NO_ERROR != Result )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      AttrRec: %016I64X   Corrupted", 
                    Attribute->AttributeInodeReference.FileId );
                OutMessage( Mess );
#endif
                continue;
            }

            //
            // Skip if this one reference other Base record
            //
            if ( PrevAttrMftIndex.FileId == AttrMftIndex.FileId )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      AttrRec: %016I64X   BaseRec: %016I64X but not %016I64X", 
                    Attribute->AttributeInodeReference.FileId, 
                    MftRecord->BaseFileRecord.FileId,
                    FileCtx->FId.FileId );
                OutMessage( Mess );
#endif
                continue;
            }

            Result = NtfsParseAttributes( 
                        (PBYTE)MftRecord + MftRecord->AttributeOffset, 
                        VolumeCtx->NtfsData.BytesPerFileRecordSegment - MftRecord->AttributeOffset, // RecordLength - ...
                        ListDepth + 1,
                        FileCtx );

            if ( NO_ERROR != Result )
            {
                continue;
            }

            PrevAttrMftIndex.FileId = AttrMftIndex.FileId;

        } // for

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"NtfsParseAttributesList(). Exception !!!", Result, ERR_OS );
    }


    if ( MftRecordsBuffer )
    {
        free( MftRecordsBuffer );
    }

    //FreeFileCtx( ListStreamCtx );


    return Result;

} // end of NtfsParseAttributesList

//
// Parse Attributes set (Base Mft record or Attributes list)
//
int
NtfsParseAttributes( 
    IN  PBYTE                   AttributeBuffer,
    IN  DWORD                   BufferLength,
    IN  int                     ListDepth,
    OUT PDEFRAG_FILE_CONTEXT    FileCtx )
{
    int                         Result = DFRG_ERROR_SKIP; // ERROR_INVALID_PARAMETER;

    DWORD                       AttributeOffset = 0;
    PMFT_ATTRIBUTE              Attribute = NULL;
    PMFT_NONRESIDENT_ATTRIBUTE  NonResident;
    PMFT_RESIDENT_ATTRIBUTE     Resident;

    PDEFRAG_STREAM_CONTEXT      ListStreamCtx = NULL;
    int                         AttrListsNum = 0;
    PBYTE                       ListBuffer = NULL;


    __try
    {
        //
        // Enumerate file's attributes. Skip attributes lists for now.
        //
        for ( ; AttributeOffset < BufferLength; AttributeOffset += Attribute->Length ) 
        {
            Attribute = (PMFT_ATTRIBUTE)(AttributeBuffer + AttributeOffset);

            if ( AttributeOffset + 4 <= BufferLength &&
                MftAttrRecordEnd == Attribute->AttributeType )
            {
                //
                // Last attribute
                //
                Result = NO_ERROR;
                break;
            }

            if ( AttributeOffset + 4 > BufferLength ||
                Attribute->Length < 3 ||
                AttributeOffset + Attribute->Length > BufferLength ) 
            {
                //
                // Corrupted record
                //
                Result = DFRG_ERROR_FS_DATA_CORRUPTED;
                __leave;
            }

            Result = NtfsExploreAttribute( 
                            Attribute, 
                            ListDepth, 
                            FileCtx, 
                            &AttrListsNum );
            if ( NO_ERROR != Result && DFRG_ERROR_SKIP != Result )
            {
                __leave;
            }

        } // for ( Attributes


        PBYTE                   AttributesList;
        DWORD                   ListLength = 0;

        //
        // Enumerate file's attributes lists.
        // Delayed it up to this point because $Mft (#0) record could contain attributes lists
        // and we should to now it's ($Mft) ClustersMap to read this lists.
        // May be for other records attributes lists could be processed in place
        // as Mft ClustersMap is already known when they are analyzed.
        //
        AttributeOffset = 0;
        Attribute = NULL;

        for ( int ListNum = 0; 
            AttributeOffset < BufferLength  &&  ListNum < AttrListsNum; 
            AttributeOffset += Attribute->Length ) 
        {
            Attribute = (PMFT_ATTRIBUTE)(AttributeBuffer + AttributeOffset);

            if ( MftAttrRecordEnd == Attribute->AttributeType )
            {
                //
                // Very strange as we should exit from loop early - on ListNum == AttrListsNum
                //
                break;
            }
            if ( Attribute->AttributeType != MftAttrList )
            {
                continue;
            }

            if ( !Attribute->Nonresident ) // Resident attributes list
            {
                Resident = (PMFT_RESIDENT_ATTRIBUTE)Attribute;

                AttributesList = (PBYTE)Resident + Resident->ValueOffset;
                ListLength     = Resident->ValueLength;
            }
            else // NonResident attributes list
            {
                //
                // TODO: this brunch is not tested yet
                //
                NonResident = (PMFT_NONRESIDENT_ATTRIBUTE)Attribute;

                //
                // Read attributes list records
                //
                PDEFRAG_VOLUME_CONTEXT  VolumeCtx = FileCtx->VolumeCtx;

                ListStreamCtx = (PDEFRAG_STREAM_CONTEXT) malloc( sizeof(DEFRAG_STREAM_CONTEXT) );
                if ( !ListStreamCtx )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }
                memset( ListStreamCtx, 0, sizeof(DEFRAG_STREAM_CONTEXT) );

                Result = NtfsGetClustersMapFromMft( 
                            NonResident, 
                            ListStreamCtx, 
                            VolumeCtx );
#ifdef _DEBUG
                OutMessage( L"NonResident Attributes List ClustersMap:" );
                PrintClustersMap( ListStreamCtx->ClustersMap );
#endif
                if ( NO_ERROR != Result )
                {   
                    __leave;
                }

                //
                // TODO: check if DataSize > DWORD
                //
                ListLength = (DWORD)NonResident->DataSize;

                if (  ListBuffer )
                {
                    free( ListBuffer );
                }

                ListBuffer = (PBYTE)malloc( ListLength );
                if ( !ListBuffer )
                {
                    __leave;
                }

                //DWORD           BytesRead = 0;
                LARGE_INTEGER   Offset;

                Offset.QuadPart = NonResident->StartingVcn * VolumeCtx->NtfsData.BytesPerCluster; // AttributeList->StartingVcn;

                Result = DfrgReadStreamRaw( 
                            VolumeCtx->MftFileCtx->DefaultStream, 
                            &Offset, 
                            ListBuffer, 
                            ListLength, 
                            TRUE,
                            (PULONG)&ListLength );
                if ( NO_ERROR != Result )
                {
                    __leave;
                }

                AttributesList = ListBuffer;

                FreeFileCtx( ListStreamCtx );

            } // if ( Resident

            //Result = 
            NtfsParseAttributesList( 
                            AttributesList,
                            ListLength,
                            ListDepth,
                            FileCtx );

            ListNum++;

        } // for ( ListNum

#ifdef _DEBUG
        static int cnt = 0;
        if ( !ListDepth )
        {
            if ( AttrListsNum || cnt<16 )
            {
                //WCHAR                       Mess[MAX_PATH];
                //swprintf( Mess, L"Record %d", cnt );
                //OutMessage( Mess );
                PrintFileCtx( FileCtx );
                cnt++;
            }
        }
#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"NtfsParseAttributes(). Exception !!!", Result, ERR_OS );
    }

    if ( ListBuffer )
    {
        free( ListBuffer );
    }

    FreeStreamCtx( ListStreamCtx );

    return Result;

} // end of NtfsParseAttributes


//
// Get File info from Mft record
//
int
NtfsGetFileInfoFromMftRec( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PMFT_FILE_RECORD_HEADER MftRecord,
    IN  ULONGLONG               RecordIndex,
    OUT PDEFRAG_FILE_CONTEXT    *OutFileCtx )
{
    int                         Result = DFRG_ERROR_SKIP; // ERROR_INVALID_PARAMETER;
    //MFT_RECORD_HEADER           MftHeader = MftRecord->MftHeader;
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;

#ifdef _DEBUG
    WCHAR                   Mess[MAX_PATH];
#endif


    __try
    {
        //
        // Check if record is in use
        //
        if ( !(MftRecord->Flags & 1) ) 
        {
            __leave;
        }

        //
        // Check if record is the first record for the file
        //
        if ( MftRecord->BaseFileRecord.MftIndexLowPart || 
            MftRecord->BaseFileRecord.MftIndexHighPart ) 
        {
            __leave;
        }

        //
        // Check offsets
        // TODO: Check other fields if required (Flags, MFTRecordIndexber, etc.)
        //
        DWORD RecordLength      = VolumeCtx->NtfsData.BytesPerFileRecordSegment;

        if ( RecordLength < MftRecord->AttributeOffset ||
            RecordLength < MftRecord->BytesInUse ) 
        {
            //
            // Corrupted record
            //
            Result = DFRG_ERROR_FS_DATA_CORRUPTED;
            __leave;
        }

        //
        // Allocate file context structure
        //
        FileCtx = (PDEFRAG_FILE_CONTEXT) malloc( sizeof(DEFRAG_FILE_CONTEXT) );
        if ( !FileCtx )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        memset( FileCtx, 0, sizeof(DEFRAG_FILE_CONTEXT) );
        FileCtx->VolumeCtx = VolumeCtx;

        //
        // Collect file and streams data into file context
        //
        RecordLength = min( RecordLength+4, MftRecord->BytesInUse );

        //
        // File ID is made up from a MFT index (48 lower bits) and a sequence number (16 upper bits) 
        //FileCtx->MftIndex.QuadPart = RecordIndex;
        //
        //FileCtx->FId.QuadPart = RecordIndex;
        //FileCtx->FId.HighPart &= 0x0000FFFF;
        //FileCtx->FId.HighPart |= ((ULONG)MftRecord->SequenceNumber << 16);

        FileCtx->FId.FileId = RecordIndex;
        FileCtx->FId.SequenceNumber = MftRecord->SequenceNumber;

        //VolumeCtx->Statistic.FilesNum.QuadPart++;
        if ( (MftRecord->Flags & 2) )
        {
            FileCtx->DefragFlags |= DEFRAG_FILE_DIRECTORY;
            //VolumeCtx->Statistic.DirsNum.QuadPart++;
        }

        //
        // Enumerate file's attributes and parse them.
        //
        Result = NtfsParseAttributes( 
                    (PBYTE)MftRecord + MftRecord->AttributeOffset, 
                    RecordLength - MftRecord->AttributeOffset, 
                    0,
                    FileCtx );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
#ifdef _DEBUG
        swprintf( Mess, L"NtfsGetFileInfoFromMft(). Exception on record %I64d !!!", RecordIndex );
        DislpayErrorMessage( Mess, Result, ERR_OS );
#else
        DislpayErrorMessage( L"NtfsGetFileInfoFromMft(). Exception", Result, ERR_OS );
#endif
    }

    if ( NO_ERROR != Result )
    {
        FreeFileCtx( FileCtx );
        FileCtx = NULL;
    }

    *OutFileCtx = FileCtx;


    return Result;

} // end of NtfsGetFileInfoFromMft

//
// Get File info from Mft record
//
int
NtfsGetFileList( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER; // DFRG_ERROR_SKIP; // 

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL, MftFileCtx = NULL;
    LARGE_INTEGER               ByteOffset = { 0 };

    LARGE_INTEGER               RecordIndex = { 0 };
    LARGE_INTEGER               ReadLength = { 0 };
    DWORD                       i;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];
#endif

    __try
    {
        if ( VolumeCtx->NtfsData.MftValidDataLength.HighPart )
        {
            OutMessage( L"Too long MFT" );
            Result = ERROR_BUFFER_OVERFLOW;
            __leave;
        }

        //ULONG MftSize = VolumeCtx->NtfsData.MftValidDataLength.LowPart;
        //ULONG Round = MftSize % VolumeCtx->NtfsData.BytesPerSector;
        //if ( Round )
        //{
        //    MftSize += VolumeCtx->NtfsData.BytesPerSector - Round;
        //}

        DWORD                   BytesPerMftRecord = VolumeCtx->NtfsData.BytesPerFileRecordSegment;
        PMFT_FILE_RECORD_HEADER MftRecord;
        DWORD                   DataLength;

        //
        // Read first Fmt record ($Mft)
        //
        ByteOffset.QuadPart = VolumeCtx->NtfsData.MftStartLcn.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
        DataLength = BytesPerMftRecord;

        Result = DfrgReadVolumeData( 
                    VolumeCtx, 
                    &ByteOffset, 
                    VolumeCtx->FileRecordsBuffer, 
                    DataLength );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

#ifdef _DEBUG
        OutMessage( L"MftFileCtx:" );
#endif

        MftRecord = (PMFT_FILE_RECORD_HEADER)VolumeCtx->FileRecordsBuffer;

        Result = NtfsCookMftRecord( VolumeCtx, MftRecord ); 
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        Result = NtfsGetFileInfoFromMftRec( VolumeCtx, MftRecord, (ULONGLONG)0, &MftFileCtx ); 
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        VolumeCtx->MftFileCtx = MftFileCtx;

        //
        // Read $Mft and build file list
        //
        DWORD           BytesRead;
        DWORD           RecordsInBuf;
        
        DataLength = FILE_RECS_BUFFER_SIZE;
        RecordsInBuf = (DataLength/BytesPerMftRecord);

        //
        // Mark progress
        //
        VolumeCtx->JobCtx->ObjectsToProcessNum = VolumeCtx->ValidRecordsNum;
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;

        VolumeCtx->Statistic.FilesNum.QuadPart = 0;
        VolumeCtx->Statistic.DirsNum.QuadPart  = 0;

        while( VolumeCtx->NtfsData.MftValidDataLength.QuadPart > ReadLength.QuadPart )
        {
            Result = DfrgReadStreamRaw( 
                        MftFileCtx->DefaultStream, 
                        &ReadLength, 
                        VolumeCtx->FileRecordsBuffer, 
                        DataLength, 
                        TRUE,
                        (PULONG)&BytesRead );
            if ( NO_ERROR != Result )
            {
                __leave;
            }

            ReadLength.QuadPart += BytesRead;

            MftRecord = (PMFT_FILE_RECORD_HEADER)VolumeCtx->FileRecordsBuffer;

            for ( i=0; 
                i<RecordsInBuf && RecordIndex.QuadPart<VolumeCtx->ValidRecordsNum.QuadPart; 
                i++, RecordIndex.QuadPart++,
                MftRecord = (PMFT_FILE_RECORD_HEADER)((PBYTE)MftRecord + BytesPerMftRecord) )
            {
                FileCtx = NULL;

                if ( !ShouldContinue( VolumeCtx->JobCtx ) )
                {
                    OutMessage( L"Ntfs Get List. Job Stopped" );
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

                //
                // Check for reserved records
                //
                //if ( //!RecordIndex.QuadPart || // Mft record - already got it 
                //    RecordIndex.QuadPart >= 12 && RecordIndex.QuadPart <= 23 )
                //{
                //    continue;
                //}

                Result = NtfsCookMftRecord( VolumeCtx, MftRecord ); 
                if ( NO_ERROR != Result )
                {
                    //if ( DFRG_ERROR_SKIP != Result ) __leave;
                    continue;
                }

                Result = NtfsGetFileInfoFromMftRec( VolumeCtx, MftRecord, RecordIndex.QuadPart, &FileCtx ); 
                if ( NO_ERROR != Result )
                {
                    if ( DFRG_ERROR_SKIP != Result ) 
                    {
                        __leave;
                    }
                    continue;
                }

                //
                // Mark progress
                //
                VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart++;

                //
                // Check attributes, set flags and update display
                //
                if ( !DfrgMarkupFile( VolumeCtx->JobCtx, FileCtx ) )
                {
                    FreeFileCtx( FileCtx );
                    continue;
                }

                if ( !DfrgAddFileToLists( VolumeCtx, FileCtx ) )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    OutMessage( L"NtfsGetFileList(). AddFileToLists - ERROR_NOT_ENOUGH_MEMORY" );

                    FreeFileCtx( FileCtx );
                    __leave;
                }
            } // for
        } // while

#ifdef _DEBUGS
        OutMessage( L"NtfsGetFileList OK" );
#endif

        //if ( DFRG_ERROR_SKIP == Result )
            Result = NO_ERROR;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"NtfsGetFileList(). Exception !!!", Result, ERR_OS );
#ifdef _DEBUG
        swprintf( Mess, L"   MftValidDataLength %I64d,  ReadLength %I64d,  RecordIndex %I64d", 
            VolumeCtx->NtfsData.MftValidDataLength.QuadPart, ReadLength.QuadPart, RecordIndex.QuadPart );
        OutMessage( Mess );
        if ( FileCtx )
        {
            swprintf( Mess, L"   FileCtx %p : '%ls'", 
                FileCtx, FileCtx->FName ? FileCtx->FName : L"NULL" );
            OutMessage( Mess );
        }
#endif
    }

    //FreeFileCtx( MftFileCtx );


#ifdef _DEBUGS
    OutMessage( L"NtfsGetFileList exit" );
#endif

    return Result;

} // end of NtfsGetFileList

