/*
    Fat disk analysis module 

    Module name:

        NtfsUtils.cpp

    Abstract:

        Fat utilities module. 
        Defines functions, used for FAT 12/16/32/Ex disk analysis 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FatUtils.cpp,v 1.4 2009/12/23 13:01:47 dimas Exp $
    $Log: FatUtils.cpp,v $
    Revision 1.4  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.3  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.2  2009/11/30 12:48:36  dimas
    1. Smart modes improved.
    2. FAT processing improved.

    Revision 1.1  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

*/


#include "stdafx.h"



unsigned char 
FatShortNameCheckSum(
    BYTE       *Name )
{
	short           FcbNameLen;
	unsigned char   Sum = 0;


	for ( FcbNameLen=11; FcbNameLen!=0; FcbNameLen-- ) 
    {
        //
		// NOTE: The operation is an unsigned char rotate right
        //
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *Name++;
	}

	return Sum;

} // end of FatShortNameCheckSum


#define     FAT12_MAX_CLUSTER       0x0FF8
#define     FAT16_MAX_CLUSTER       0xFFF8
#define     FAT32_MAX_CLUSTER   0x0FFFFFF8 

DWORD
FatGetNextCluster( 
    IN  PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    IN  DWORD                       Cluster )
{
    DWORD                       NextCluster = 0;
    DWORD                       FatOffset;


    switch ( VolumeCtx->DiskType )
    {
    case Fat32Disk:

        if ( Cluster >= FAT32_MAX_CLUSTER )
        {
            break;
        }

        FatOffset = Cluster * 4;

        NextCluster = (*((DWORD *)(VolumeCtx->FatBuffer+FatOffset))) & 0x0FFFFFFF;

        if ( NextCluster >= FAT32_MAX_CLUSTER )
        {
            NextCluster = 0;
        }

        break;

    case Fat16Disk:

        if ( Cluster >= FAT16_MAX_CLUSTER )
        {
            break;
        }

        FatOffset = Cluster * 2;

        NextCluster = *((WORD *)(VolumeCtx->FatBuffer+FatOffset));

        if ( NextCluster >= FAT16_MAX_CLUSTER )
        {
            NextCluster = 0;
        }

        break;

    case Fat12Disk:

        if ( Cluster >= FAT12_MAX_CLUSTER )
        {
            break;
        }

        FatOffset = Cluster + Cluster / 2;

        NextCluster = *((WORD *)(VolumeCtx->FatBuffer+FatOffset));

        if ( Cluster & 0x1 ) 
        {
            NextCluster = NextCluster >> 4;	    // Cluster number is ODD 
        }
        else
        {
            NextCluster = NextCluster & 0x0FFF;	// Cluster number is EVEN
        }

        if ( NextCluster >= FAT12_MAX_CLUSTER )
        {
            NextCluster = 0;
        }

        break;
    }


    return NextCluster;

} // end of FatGetNextCluster


int
FatBuildClustersMap( 
    IN      PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    IN      DWORD                       FirstCluster,
    OUT     PRETRIEVAL_POINTERS_BUFFER  *ClustersMap,
    OUT     DWORD                       *ClustersNum )
{
    int                         Result = ERROR_INVALID_PARAMETER; 
    DWORD                       Cluster, NextCluster, ExtentCount = 0;


    *ClustersMap = NULL;
    *ClustersNum = 0;

    //
    // Get extents number
    //
    Cluster = FirstCluster;

    //
    // Note: for 0 length file FirstCluster == 0
    //
    while ( Cluster >= 2 )
    {
        ExtentCount++;

        while( (NextCluster = FatGetNextCluster( VolumeCtx, Cluster )) == Cluster+1 )
        {
            Cluster = NextCluster;
        }
        Cluster = NextCluster;
    }

    if ( !ExtentCount )
    {
        return Result;
    }

    //
    // Allocate ClustersMap buffer
    //
    *ClustersMap = 
        (PRETRIEVAL_POINTERS_BUFFER) malloc( sizeof(RETRIEVAL_POINTERS_BUFFER) + (ExtentCount-1) * sizeof(LARGE_INTEGER) * 2 );

    if ( !(*ClustersMap) )
    {
        Result = ERROR_NOT_ENOUGH_MEMORY; 
        return Result;
    }

    //
    // Build ClustersMap
    //
    Cluster = FirstCluster;

    (*ClustersMap)->ExtentCount = ExtentCount;
    (*ClustersMap)->StartingVcn.QuadPart = 0;

    DWORD   ClustersCount = 0;

    for ( DWORD i=0; i<ExtentCount && Cluster>=2; i++ )
    {
        //
        // Cluster number - Adjusted from FAT to API ( - 2 )
        //
        (*ClustersMap)->Extents[i].Lcn.QuadPart = Cluster - 2;

        while( (NextCluster = FatGetNextCluster( VolumeCtx, Cluster )) == Cluster+1 )
        {
            ClustersCount++;
            Cluster = NextCluster;
        }
        ClustersCount++;
        Cluster = NextCluster;

        (*ClustersMap)->Extents[i].NextVcn.QuadPart = ClustersCount;
    }


    *ClustersNum = ClustersCount;

    Result = NO_ERROR;


    return Result;

} // end of FatBuildClustersMap

#if 0
typedef short CSHORT;

typedef struct _TIME_FIELDS {
    CSHORT Year;
    CSHORT Month;
    CSHORT Day;
    CSHORT Hour;
    CSHORT Minute;
    CSHORT Second;
    CSHORT Milliseconds;
    CSHORT Weekday;
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );
#endif


int
FatGetFileInfoFromDirRec( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PFAT_DIR_RECORD         *DirRecord, 
    OUT PDEFRAG_FILE_CONTEXT    *OutFileCtx )
{

    int                         Result = NO_ERROR; // ERROR_INVALID_PARAMETER; 

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_STREAM_CONTEXT      StreamCtx = NULL;

    PFAT_DIR_RECORD             ShortDirRec = NULL;
    PFAT_LONG_DIR_RECORD        LongDirRec;

    DWORD                       FirstCluster;

#ifdef _DEBUG
        WCHAR       Mess[MAX_PATH];
#endif


    __try
    {
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
        // Allocate stream context struct
        //
        StreamCtx = (PDEFRAG_STREAM_CONTEXT) malloc( sizeof(DEFRAG_STREAM_CONTEXT) );
        if ( !StreamCtx )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        memset( StreamCtx, 0, sizeof(DEFRAG_STREAM_CONTEXT) );

        StreamCtx->FileCtx = FileCtx;

        FileCtx->DefaultStream = 
            FileCtx->FirstStream = StreamCtx;

        if ( !DirRecord )
        {
            //
            // Special processing for Root dir
            //
            FileCtx->DefragFlags |= (DEFRAG_FILE_DIRECTORY | DEFRAG_FILE_ROOT_DIR);
            FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_EXCLUDED);

            if ( VolumeCtx->DiskType == Fat32Disk )
            {
                FirstCluster = VolumeCtx->DosData.RootDirOffset; // the first cluster of root dir on fat32
            }
            else
            {
                FileCtx->TotalClustersNum.QuadPart = 
                    ( VolumeCtx->DosData.RootDirLength * VolumeCtx->NtfsData.BytesPerSector +
                    VolumeCtx->NtfsData.BytesPerCluster - 1) / VolumeCtx->NtfsData.BytesPerCluster;

                FileCtx->FId.FileId = -1;

                Result = NO_ERROR;

                __leave;
            }
        }
        else
        {
            BOOLEAN     IsLongName = FALSE;
            BYTE        NameCheckSum = 0;
            BOOLEAN     IsFile = FALSE;
            DWORD       LongNameLength = 0;

            ShortDirRec = *DirRecord;
            LongDirRec = (PFAT_LONG_DIR_RECORD)ShortDirRec;

            for ( ;; LongDirRec++, ShortDirRec++ )
            {
                if ( !LongDirRec->Ordinal )
                {
                    //
                    // No more entries
                    //
                    ShortDirRec = NULL;
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

                if ( LongDirRec->Ordinal == 0xE5 )
                {
                    //
                    // Empty record
                    //
                    IsLongName = FALSE; // orphase
                }
                else if ( (LongDirRec->Attributes & FILE_ATTRIBUTE_LONG_NAME_MASK) == FILE_ATTRIBUTE_LONG_NAME )
                {
                    //
                    // Long name record
                    //
                    if ( !IsLongName ) 
                    {
                        if ( (LongDirRec->Ordinal & LAST_LONG_ENTRY) ) // last long name extent
                        {
                            //LongNameLength = (LongDirRec->Ordinal & (~LAST_LONG_ENTRY)) * FAT_LONGNAME_ENTRY_LENGTH + 1;
                            LongNameLength = (LongDirRec->Ordinal & (~LAST_LONG_ENTRY));
                            IsLongName = TRUE;
                            NameCheckSum = LongDirRec->CheckSum;
                        }
                    }
                    else
                    {
                        if ( NameCheckSum != LongDirRec->CheckSum )
                        {
                            IsLongName = FALSE; // orphase
#ifdef _DEBUGW
                            wsprintf( Mess, L"   Wrong LongNameCheckSum 1. %02X != %02X", 
                                (ULONG)NameCheckSum, (ULONG)LongDirRec->CheckSum );
                            OutMessage( Mess );
#endif
                        }
                    }
                }
                else if ( (LongDirRec->Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VOLUME_ID)) == 0x00 )
                {
		            //   Found a file
                    IsFile = TRUE;
                }
	            else if ( (LongDirRec->Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VOLUME_ID)) == FILE_ATTRIBUTE_DIRECTORY )
                {
		            //   Found a directory
                    IsFile = TRUE;
                }
	            else if ( (LongDirRec->Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VOLUME_ID)) == FILE_ATTRIBUTE_VOLUME_ID )
                {
		            //   Found a volume label
                    IsLongName = FALSE; // orphase
                }
	            else
                {
		            // Found an invalid directory entry
                    IsLongName = FALSE; // orphase
                }


                if ( IsFile )
                {
                    //
                    // Grabe attributes
                    //
                    FileCtx->Attributes = ShortDirRec->Attributes;
/*
                    VolumeCtx->Statistic.FilesNum.QuadPart++;
	                if ( (ShortDirRec->Attributes & FILE_ATTRIBUTE_DIRECTORY) )
                    {
                        VolumeCtx->Statistic.DirsNum.QuadPart++;
                    }
*/
                    //
                    // TODO: Convert DOS file times to WinNT (System)
                    //
#ifndef DEFRAG_NATIVE_APP
                    FILETIME        FileTime;

                    DosDateTimeToFileTime( ShortDirRec->CreateDate, ShortDirRec->CreateTime, &FileTime );
                    FileCtx->CreationTime.HighPart = FileTime.dwHighDateTime;
                    FileCtx->CreationTime.LowPart  = FileTime.dwLowDateTime;

                    DosDateTimeToFileTime( ShortDirRec->WriteDate, ShortDirRec->WriteTime, &FileTime );
                    FileCtx->ChangeTime.HighPart = FileTime.dwHighDateTime;
                    FileCtx->ChangeTime.LowPart  = FileTime.dwLowDateTime;

                    DosDateTimeToFileTime( ShortDirRec->LastAccessDate, 0, &FileTime );
                    FileCtx->LastAccessTime.HighPart = FileTime.dwHighDateTime;
                    FileCtx->LastAccessTime.LowPart  = FileTime.dwLowDateTime;
#else 
#if 0
                    //
                    // TODO
                    //
                    PDOS_FILE_DATE  DosDate;
                    PDOS_FILE_TIME  DosTime;
                    TIME_FIELDS     TimeFields;
                    LARGE_INTEGER   Time;

                    DosDate = (PDOS_FILE_DATE)&(ShortDirRec->CreateDate);
                    DosTime = (PDOS_FILE_TIME)&(ShortDirRec->CreateTime);

                    TimeFields.Year = DosDate->year + 1980 - 1601;

                    RtlTimeFieldsToTime( &TimeFields, &Time );

                    FileCtx->CreationTime = Time;
#endif
                    FileCtx->CreationTime.HighPart   = ShortDirRec->CreateDate;
                    FileCtx->CreationTime.LowPart    = ShortDirRec->CreateTime;
                    FileCtx->ChangeTime.HighPart     = ShortDirRec->WriteDate;
                    FileCtx->ChangeTime.LowPart      = ShortDirRec->WriteTime;
                    FileCtx->LastAccessTime.QuadPart = ShortDirRec->LastAccessDate;
#endif

                    if ( !FileCtx->ChangeTime.QuadPart )
                    {
                        //
                        // TODO: byModify is not supported
                        //
                        FileCtx->ChangeTime = FileCtx->CreationTime;
                    }

                    if ( !FileCtx->LastAccessTime.QuadPart )
                    {
                        //
                        // TODO: byAccess is not supported
                        //
                        FileCtx->LastAccessTime = FileCtx->ChangeTime;
                    }

                    //
                    // Note. DirSize == 0
                    //
                    FileCtx->DefaultStream->DataSize.QuadPart = ShortDirRec->FileSize;

                    //
                    // Get file name
                    //
                    DWORD   FileNameLength, i;
                    USHORT  NameType = NO_NAME;

                    if ( IsLongName )
                    {
                        //
                        // Build long name. Check validity first.
                        //
                        if ( NameCheckSum == FatShortNameCheckSum( ShortDirRec->ShortName ) )
                        {
                            //FileCtx->NameType = LONG_NAME;
                            NameType = LONG_NAME;
                            FileCtx->DefragFlags2 |= DEFRAG_FILE2_LONG_NAME;

                            FileNameLength = (LongNameLength * FAT_LONGNAME_ENTRY_LENGTH + 1) * sizeof(WCHAR);

                            FileCtx->FName = (WCHAR*)malloc( FileNameLength );
                            if ( !FileCtx->FName )
                            {
                                Result = ERROR_NOT_ENOUGH_MEMORY;
                                __leave;
                            }

                            memset( FileCtx->FName, 0, FileNameLength );

                            for( i=1, FileNameLength=0; i<=LongNameLength; i++ )
                            {
                                memcpy( FileCtx->FName+FileNameLength, (LongDirRec-i)->Name1, 5 * sizeof(WCHAR) );
                                FileNameLength += 5;
                                memcpy( FileCtx->FName+FileNameLength, (LongDirRec-i)->Name2, 6 * sizeof(WCHAR) );
                                FileNameLength += 6;
                                memcpy( FileCtx->FName+FileNameLength, (LongDirRec-i)->Name3, 2 * sizeof(WCHAR) );
                                FileNameLength += 2;
                            }
                        }
                        else
                        {
#ifdef _DEBUG
                            swprintf( Mess, L"   Wrong LongNameCheckSum 1. %02X != %02X", 
                                (ULONG)NameCheckSum, (ULONG)FatShortNameCheckSum( ShortDirRec->ShortName ) );
                            OutMessage( Mess );
#endif
                            IsLongName = FALSE;
                        }
                    }

                    if ( !IsLongName )
                    {
                        //
                        // Build short name
                        //
                        //FileCtx->NameType = SHORT_NAME;
                        NameType = SHORT_NAME;
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_SHORT_NAME;
                        FileNameLength = 13 * sizeof(WCHAR);

                        FileCtx->FName = (WCHAR*)malloc( FileNameLength );
                        if ( !FileCtx->FName )
                        {
                            Result = ERROR_NOT_ENOUGH_MEMORY;
                            __leave;
                        }

                        memset( FileCtx->FName, 0, FileNameLength );

                        char    *NamePtr = (char*)FileCtx->FName;

                        for( i=0; i<8; i++, NamePtr+=2 )
                        {
                            if ( ShortDirRec->ShortName[i] == ' ' )
                            {
                                break;
                            }
                            *NamePtr = ShortDirRec->ShortName[i];
                        }

                        if ( ShortDirRec->Extention[0] != ' ' )
                        {
                            *NamePtr = '.';
                            NamePtr += 2;

                            for( i=0; i<3; i++, NamePtr+=2 )
                            {
                                if ( ShortDirRec->Extention[i] == ' ' )
                                {
                                    break;
                                }
                                *NamePtr = ShortDirRec->Extention[i];
                            }
                        }
                    }

                    break;
                }

            } // while

            //
            // Get files first cluster number
            //
            FirstCluster = ShortDirRec->FirstClusterLow;
            if ( Fat32Disk == VolumeCtx->DiskType )
            {
                *(((WORD*)&FirstCluster)+1) = ShortDirRec->FirstClusterHigh;
            }

        } // if ( !RootDir

        FileCtx->FId.FileId = FirstCluster; 

        //
        // Build files cluster map
        //
        FileCtx->TotalClustersNum.QuadPart = 0;

        if ( FirstCluster >= 2 )
        {
            Result = FatBuildClustersMap( 
                                    VolumeCtx, 
                                    FirstCluster, 
                                    &(FileCtx->DefaultStream->ClustersMap), 
                                    (DWORD*)&(FileCtx->TotalClustersNum.LowPart) );
            if ( NO_ERROR != Result )
            {
#ifdef _DEBUG
                swprintf( Mess, L"   FatBuildClustersMap error on '%ls'", 
                    FileCtx->FName ); 
                OutMessage( Mess );
#endif
                __leave;
            }
        }

#ifdef _DEBUG
        //
        // Validate length field
        //
        WCHAR       Mess[MAX_PATH];

        if ( DirRecord && ShortDirRec )
        {
            if ( (FileCtx->Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VOLUME_ID)) == 0x00 )
            {
                if ( FileCtx->TotalClustersNum.QuadPart !=
                    (FileCtx->DefaultStream->DataSize.QuadPart + VolumeCtx->NtfsData.BytesPerCluster - 1) / // ShortDirRec->FileSize
                        VolumeCtx->NtfsData.BytesPerCluster )
                {
                    swprintf( Mess, L"   GetFileInfoFromDirRec '%ls' size %d != %d", 
                        FileCtx->FName,
                        FileCtx->TotalClustersNum.QuadPart, 
                        (ShortDirRec->FileSize + VolumeCtx->NtfsData.BytesPerCluster - 1) / VolumeCtx->NtfsData.BytesPerCluster );
                    OutMessage( Mess );
                    //Result = DFRG_ERROR_FS_DATA_CORRUPTED;
                    //__leave;
                }
            }
        }
#endif

        Result = NO_ERROR;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"FatGetFileInfoFromDirRec(). Exception !!!", Result, ERR_OS );
    }


    if ( NO_ERROR != Result )
    {
        FreeFileCtx( FileCtx );
        FileCtx = NULL;

        ShortDirRec = NULL;
    }

    *OutFileCtx = FileCtx;

    if ( ShortDirRec )
    {
        ShortDirRec++;
    }

    if ( DirRecord )
    {
        *DirRecord = ShortDirRec;
    }


    return Result;

} // end of FatGetFileInfoFromDirRec


int
FatReadDirectory( 
    IN  PDEFRAG_FILE_CONTEXT    DirCtx,
    OUT PBYTE                   *DirBuffer,
    OUT DWORD                   *DirLength )
{
    int                         Result = ERROR_INVALID_PARAMETER; 

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = DirCtx->VolumeCtx;

    __try
    {
        LARGE_INTEGER   Offset = { 0 };
        DWORD           BytesRead, Length;

        *DirLength = 0;

        if ( (DirCtx->DefragFlags & DEFRAG_FILE_ROOT_DIR) &&
            Fat32Disk != VolumeCtx->DiskType )
        {
            Length = VolumeCtx->DosData.RootDirLength * VolumeCtx->NtfsData.BytesPerSector;

            (*DirBuffer) = (PBYTE) malloc( Length + sizeof(FAT_DIR_RECORD) );
            if ( !(*DirBuffer) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            Offset.LowPart = VolumeCtx->DosData.RootDirOffset * VolumeCtx->NtfsData.BytesPerSector; 
            Result = DfrgReadVolumeData( 
                            VolumeCtx, 
                            &Offset,
                            (*DirBuffer), 
                            Length );
            if ( NO_ERROR != Result )
            {
                __leave;
            }

        }
        else
        {
            Length = (DWORD)DirCtx->TotalClustersNum.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;

            (*DirBuffer) = (PBYTE) malloc( Length + sizeof(FAT_DIR_RECORD) );
            if ( !(*DirBuffer) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            Result = DfrgReadStreamRaw( 
                        DirCtx->DefaultStream, 
                        &Offset, 
                        (*DirBuffer), 
                        Length, 
                        TRUE,
                        (PULONG)&BytesRead   );
            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }

        //
        // Set end of dir marker
        //
        memset( (*DirBuffer) + Length, 0, sizeof(FAT_DIR_RECORD) );

        *DirLength = Length / sizeof(FAT_DIR_RECORD);   

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"FatReadDirectory(). Exception !!!", Result, ERR_OS );
    }


    return Result;

} // end of FatReadDirectory


int
FatScanDirectory( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PDEFRAG_FILE_CONTEXT    DirCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER; 

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;

    PFAT_DIR_RECORD             DirRecord;
    PBYTE                       DirBuffer = NULL;
    DWORD                       DirLength = 0;
    BOOLEAN                     ShouldInclude;

    __try
    {
#ifdef _DEBUG
        WCHAR       Mess[MAX_PATH];

        OutMessage( L"" );
        swprintf( Mess, L"FatScanDirectory '%ls'", 
            (DirCtx->DefragFlags & DEFRAG_FILE_ROOT_DIR)?L"Root":(DirCtx->FName?DirCtx->FName:L"NULL") );
        OutMessage( Mess );

        //if ( DirCtx->FName && !wcscmp( DirCtx->FName, L"TestCases" ) )
        //{
        //    OutMessage( L"Bad dir :)" );
        //}
#endif
        //
        // Read directory
        //
        Result = FatReadDirectory( DirCtx, &DirBuffer, &DirLength );
        if ( NO_ERROR != Result ) 
        {
            __leave;
        }

        DirRecord = (PFAT_DIR_RECORD)DirBuffer;

#ifdef _DEBUGW
        swprintf( Mess, L"   DirLength %d First entry '%c'", 
            DirLength, (int)DirRecord->ShortName[0] );
        OutMessage( Mess );
#endif

        for ( DWORD i=0; i<DirLength && DirRecord; i++ )
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                OutMessage( L"Fat Get List. Job Stopped" );
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            Result = FatGetFileInfoFromDirRec( VolumeCtx, &DirRecord, &FileCtx );
            if ( NO_ERROR != Result )
            {
#ifdef _DEBUGW
                swprintf( Mess, L"   FatGetFileInfoFromDirRec() failed on entry %d (from %d)", 
                    i, DirLength );
                OutMessage( Mess );
#endif
                __leave;
            }

            //
            // Mark progress
            //
            VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart += FileCtx->TotalClustersNum.QuadPart;


            FileCtx->ParentDir = DirCtx;

            ShouldInclude = DfrgMarkupFile( VolumeCtx->JobCtx, FileCtx );

#ifdef _DEBUGW
            PrintFileCtx( FileCtx );
#endif
            //
            // Check attributes, set flags and update display
            //
            if ( !ShouldInclude )
                //!wcscmp( FileCtx->FName, L"." ) ||
                //!wcscmp( FileCtx->FName, L".." ) )
            {
                FreeFileCtx( FileCtx );
                continue;
            }

            if ( !DfrgAddFileToLists( VolumeCtx, FileCtx ) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                OutMessage( L"FatScanDirectory(). AddFileToLists - ERROR_NOT_ENOUGH_MEMORY" );

                FreeFileCtx( FileCtx );
                __leave;
            }

#ifdef _DEBUGW
            DfrgOpenFile( FileCtx, FALSE );
#endif


            if ( (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
            {
                //
                // Recursivly call ourself for dirs
                //
                Result = FatScanDirectory( VolumeCtx, FileCtx );
                if ( NO_ERROR != Result && DFRG_ERROR_ABORTED != Result )
                {
                    __leave;
                }
            }

        } // for ( i

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"FatScanDirectory(). Exception !!!", Result, ERR_OS );
    }

    if ( DirBuffer )
    {
        free( DirBuffer );
    }

    return Result;

} // end of FatScanDirectory

//
// Get File info from Mft record
//
int
FatGetFileList( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER; // DFRG_ERROR_SKIP; // 

    PDEFRAG_FILE_CONTEXT        RootDirCtx = NULL;
    LARGE_INTEGER               ByteOffset = { 0 };

    PBYTE                       DirectoryBuffer = NULL;


    __try
    {
        //
        // Read Root Dir
        //
        Result = FatGetFileInfoFromDirRec( VolumeCtx, NULL, &RootDirCtx ); // Root DirRecord = NULL
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        if ( Fat32Disk == VolumeCtx->DiskType )
        {
            if ( !DfrgAddFileToLists( VolumeCtx, RootDirCtx ) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                OutMessage( L"FatGetFileList(). Add RootDirCtx - ERROR_NOT_ENOUGH_MEMORY" );

                FreeFileCtx( RootDirCtx );
                __leave;
            }
        }
        else
        {
            VolumeCtx->RootDirCtx = RootDirCtx;
        }


        //
        // Mark progress
        //
        VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart = 
            VolumeCtx->NtfsData.TotalClusters.QuadPart - VolumeCtx->NtfsData.FreeClusters.QuadPart;
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;

        VolumeCtx->Statistic.FilesNum.QuadPart = 0;
        VolumeCtx->Statistic.DirsNum.QuadPart  = 0;


        Result = FatScanDirectory( VolumeCtx, RootDirCtx );


        if ( DFRG_ERROR_SKIP == Result ||
            DFRG_ERROR_ABORTED == Result ||
            NO_ERROR == Result )
        {
#ifdef _DEBUG
            OutMessage( L"FatGetFileList OK" );
#endif
            Result = NO_ERROR;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"FatGetFileList(). Exception !!!", Result, ERR_OS );
    }

#ifdef _DEBUG
    if ( Fat12Disk == VolumeCtx->DiskType ||
        Fat16Disk == VolumeCtx->DiskType )
    {
        PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree), TRUE );
    }
    else
    {
        PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree), FALSE );
    }
    //OutMessage( L"FatGetFileList exit" );
#endif



    return Result;

} // end of FatGetFileList

