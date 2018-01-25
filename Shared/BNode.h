// =================================================
//
// BNode.h : Defines binary tree functions.
//
// =================================================

#ifndef __BNODE_H__
#define __BNODE_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

//#pragma pack( 1 )

typedef enum {
    SortModeDefault = 0,
    SortModeSize    = 1,
    SortModeName,
    SortModePath,
    SortModeCreatedDate,
    SortModeUsedDate,
    SortModeModifiedDate,
    SortModeGroup,
    SortModeLcn,
    SortModeId
} BTR_TREE_SORT_MODE;


typedef struct _BTR_NODE
{
    struct _BTR_NODE    *Parent;
    struct _BTR_NODE    *Left;
    struct _BTR_NODE    *Right;
    struct _BTR_NODE    *NextEq;
    struct _BTR_NODE    *LastEq;
    PVOID               Body;
} BTR_NODE, *PBTR_NODE, **PPBTR_NODE;


typedef void        (*FreeBodyFunc)(PVOID Body);
typedef LONGLONG    (*CompareKeysFunc)(PVOID Body1, PVOID Body2, ULONG SortMode);
typedef void        (*CopyBodyFunc)(PVOID Trg, PVOID Src);
typedef void        (*PrintBodyFunc)(PVOID Body);

typedef struct _BTR_TREE_METHODS
{
    FreeBodyFunc        FreeBody;
    CompareKeysFunc     CompareItems;
    CopyBodyFunc        CopyBody;
    PrintBodyFunc       PrintBody;
} BTR_TREE_METHODS, *PBTR_TREE_METHODS;


#define TREE_INITIALIZED    0x00000001

typedef struct _BTR_TREE
{
    struct _BTR_NODE    *Root;
    struct _BTR_NODE    *Left;
    struct _BTR_NODE    *Right;
    struct _BTR_NODE    *Current;

    LARGE_INTEGER       NodeCnt;

    BTR_TREE_SORT_MODE  SortMode;
    ULONG               Flags;

    BTR_TREE_METHODS    Methods;
    CRITICAL_SECTION    TreeSync;
} BTR_TREE, *PBTR_TREE;

//#pragma pack( )

//
// Search/Iterate direction
//
#define     LEFT_TO_RIGHT   TRUE
#define     RIGHT_TO_LEFT   FALSE

//
// Search equality condition
//
#define     _FIND_LT_    (-2)
#define     _FIND_LE_    (-1)
#define     _FIND_EQ_      0
#define     _FIND_GE_      1
#define     _FIND_GT_      2

#define     _FIND_MIN_  (-10)
#define     _FIND_MAX_    10

// =================================================
//
// Function prototypes
//
// =================================================

PBTR_NODE
AddNode(
    PBTR_TREE       Tree,
    PVOID           Body );

PBTR_NODE
FindNode(
    PBTR_TREE           Tree,
    PVOID               Template,
    BTR_TREE_SORT_MODE  Key,
    int                 Equality );

PBTR_NODE
FindFirstNode(
    PBTR_TREE       Tree,
    BOOLEAN         LeftToRight );

PBTR_NODE
FindNextNode(
    PBTR_TREE       Tree,
    BOOLEAN         LeftToRight );

void
RemoveNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node );

void
FreeNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node );

void
DeleteNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node );

void
FreeTree(
    PBTR_TREE       Tree );

void
DeleteTree(
    PBTR_TREE       Tree );

void
ReleaseTree(
    PBTR_TREE       Tree );

void
InitTree(
    PBTR_TREE           Tree,
    PBTR_TREE_METHODS   Methods,
    BTR_TREE_SORT_MODE  SortMode );



#endif  //!__BNODE_H__
