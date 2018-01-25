/*
    Defrag Engine

    Module name:

        Bnode.cpp

    Abstract:

        Defrag binary tree module. 
        Defines binary tree functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/BNode.cpp,v 1.2 2009/12/03 13:01:06 dimas Exp $
    $Log: BNode.cpp,v $
    Revision 1.2  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#include "stdafx.h"


#ifdef _DEBUG
void
PrintNode(
    PBTR_NODE       Root,
    PBTR_NODE       Node )
{
    WCHAR   Mess[MAX_PATH];


    swprintf( Mess, L"   Root %p; Node %p: parent %p  left %p  right %p  next %p  last %p", 
        Root, Node, Node->Parent, Node->Left, Node->Right, Node->NextEq, Node->LastEq );
    OutMessage( Mess );
    if ( Node->Parent )
    {
        swprintf( Mess, L"      Parent %p:  left %p  right %p", 
            Node->Parent, Node->Parent->Left, Node->Parent->Right );
        OutMessage( Mess );
    }
    if ( Node->Left )
    {
        swprintf( Mess, L"      Left %p: parent %p", 
            Node->Left, Node->Left->Parent );
        OutMessage( Mess );
    }
    if ( Node->Right )
    {
        swprintf( Mess, L"      Right %p: parent %p", 
            Node->Right, Node->Right->Parent );
        OutMessage( Mess );
    }
    if ( Node->NextEq )
    {
        swprintf( Mess, L"      NextEq %p: parent %p  next %p  last %p", 
            Node->NextEq, Node->NextEq->Parent, Node->NextEq->NextEq, Node->NextEq->LastEq );
        OutMessage( Mess );
    }
    if ( Node->LastEq )
    {
        swprintf( Mess, L"      LastEq %p: parent %p  next %p  last %p", 
            Node->LastEq, Node->LastEq->Parent, Node->LastEq->NextEq, Node->LastEq->LastEq );
        OutMessage( Mess );
    }
}
#endif


PBTR_NODE
AddNode(
    PBTR_TREE       Tree,
    PVOID           Body )
{
    PBTR_NODE   Node;

    Node = ( PBTR_NODE ) malloc( sizeof(BTR_NODE) );
    if ( !Node ) 
    {
        return NULL;
    }

    memset( Node, 0, sizeof(BTR_NODE) );

    Node->Body = Body;

    if ( !Tree->Root )
    {
        Tree->Root  = Node;
        Tree->Left  = Node;
        Tree->Right = Node;
    }
    else
    {
        PBTR_NODE   Current = Tree->Root;
        PBTR_NODE   Parent;
        LONGLONG    cmp, left = 0, right = 0;

        while ( TRUE )
        {
            cmp = Tree->Methods.CompareItems( Body, Current->Body, Tree->SortMode );

            Parent = Current;

            if ( cmp < 0 ) 
            {
                Current = Current->Left;
                left++;
                if ( !Current )
                {
                    Parent->Left = Node;
                    if ( !right )
                    {
                        Tree->Left = Node;
                    }
                    break;
                }
            }
            else if ( cmp > 0 ) 
            {
                Current = Current->Right;
                right++;
                if ( !Current )
                {
                    Parent->Right = Node;
                    if ( !left )
                    {
                        Tree->Right = Node;
                    }
                    break;
                }
            }
            else
            {
                if ( !Current->NextEq )
                {
                    Current->NextEq = Node;
                    Current->LastEq = Node;
                }
                else
                {
                    Current->LastEq->NextEq = Node;
                    Current->LastEq = Node;
                }
                break;
            }
        }

        Node->Parent = Parent;

    } // !Root

    Tree->NodeCnt.QuadPart++;

    return Node;

} // end of AddNode

//
// Find item by pattern, that is:
// - equal              if Equality = _FIND_EQ_
// - equal or grater    if Equality = _FIND_GE_
// - equal or later     if Equality = _FIND_LE_
//
PBTR_NODE
FindNode(
    PBTR_TREE           Tree,
    PVOID               Template,
    BTR_TREE_SORT_MODE  Key,
    int                 Equality )
{
    PBTR_NODE           TrgNode = NULL, PrevNode = NULL;
    PBTR_NODE           Node = Tree->Root;
    BTR_TREE_SORT_MODE  ActKey; // = (Key != SortModeDefault) ? Key : Tree->SortMode;
    LONGLONG            Cmp = 0;

/*
    if ( Key != SortModeDefault && Key != Tree->SortMode ) 
    {
        //
        // TODO.
        //
        return NULL;
    }
*/
    ActKey = Tree->SortMode;

    while ( Node )
    {
        Cmp = Tree->Methods.CompareItems( Node->Body, Template, ActKey );

        if ( Cmp == 0 )
        {/*
            if ( Key != Tree->SortMode && Key != SortModeDefault && _FIND_EQ_ == Equality )
            {
                do
                {
                    Cmp = Tree->Methods.CompareItems( Node->Body, Template, Key );
                    if ( !Cmp )
                    {
                        TrgNode = Node;
                        break;
                    }
                    Node = Node->NextEq;
                } while ( Node );
            }
            else
            {*/
                TrgNode = Node;
            //}
            break;
        }
        else if ( Cmp > 0 )
        {
            if ( _FIND_GE_ == Equality )
            {
                PrevNode = Node;
            }
            Node = Node->Left;
        }
        else
        {
            if ( _FIND_LE_ == Equality )
            {
                PrevNode = Node;
            }
            Node = Node->Right;
        }
    }

    //
    // Not exect match
    //
    if ( Cmp && _FIND_EQ_ != Equality )
    {
        TrgNode = PrevNode;
    }

    //if ( TrgNode )
        Tree->Current = TrgNode;


    return TrgNode;

} // end of FindNode


PBTR_NODE
FindFirstNode(
    PBTR_TREE       Tree,
    BOOLEAN         LeftToRight )
{
    PBTR_NODE       Current = NULL;


    if ( Tree->Root )
    {
        if ( LeftToRight )
        {
            Current = Tree->Left;
            if ( !Current )
            {
                Current = Tree->Root;
                while ( Current->Left )
                {
                    Current = Current->Left;
                }
                Tree->Left = Current;
            }
        }
        else
        {
            Tree->Current = Tree->Right;
            if ( !Current )
            {
                Current = Tree->Root;
                while ( Current->Right )
                {
                    Current = Current->Right; 
                }
                Tree->Right = Current;
            }
        }
    }

    Tree->Current = Current;


    return  Current;

} // end of FindFirstNode


PBTR_NODE
FindNextNode(
    PBTR_TREE       Tree,
    BOOLEAN         LeftToRight )
{
    PBTR_NODE       Node = NULL;
    PBTR_NODE       Current;


    if ( !Tree->Current || !Tree->Root )
    {
        return NULL;
    }

    Current = Tree->Current;

    if ( Current->NextEq )
    {
        Current = Current->NextEq;
        Tree->Current = Current;
        return  Current;
    }
    else if ( Current->Parent && Current->Parent->LastEq == Current ) // NextEq 
    {
        Current = Current->Parent;
    }

    if ( LeftToRight )
    {
        if ( Current->Right )
        {
            Node = Current->Right;
            while ( Node->Left )
            {
                Node = Node->Left;
            }
        }
        else
        {
            while ( Current->Parent )
            {
                if ( Current->Parent->Right != Current )
                {
                    Node = Current->Parent;
                    break;
                }
                Current = Current->Parent;
            }
        }
    }
    else // !LeftToRight
    {
        if ( Current->Left )
        {
            Node = Current->Left;
            while ( Node->Right )
            {
                Node = Node->Right;
            }
        }
        else 
        {
            while ( Current->Parent )
            {
                if ( Current->Parent->Left != Current )
                {
                    Node = Current->Parent;
                    break;
                }
                Current = Current->Parent;
            }
        }
    }

    Tree->Current = Node;


    return Node;

} // end of FindNextNode


//#ifdef _DEBUG
//    #define _DEBUG_REMOVE
//#endif

void
RemoveNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node )
{
#ifdef _DEBUG_REMOVE
    WCHAR   Mess[MAX_PATH];
#endif


    if ( !Node ) 
    {
        return;
    }

    Tree->Current = Tree->Left = Tree->Right = NULL;
/*
    if ( Tree->Current == Node )
    {
        Tree->Current = FindNextNode( Tree, LEFT_TO_RIGHT );
    }
    
    if ( Tree->Left == Node )
    {
        Tree->Left = FindNextNode( Tree, LEFT_TO_RIGHT );
    }
    
    if ( Tree->Right == Node )
    {
        Tree->Right = FindNextNode( Tree, RIGHT_TO_LEFT );
    }
*/

    //
    // Try to find prev Eq if any.
    //
    PBTR_NODE   PrevEq = Node->Parent;

#ifdef _DEBUGW
    if ( Node == Tree->Root )
    {
        Tree->Root = Node;
    }
#endif


    while ( PrevEq && PrevEq->NextEq != Node )
    {
        PrevEq = PrevEq->NextEq;
    }

    if ( PrevEq && PrevEq->NextEq == Node )
    {
        //
        // This node is in the midle or end of the chain of the Eq.
        //
        PrevEq->NextEq = Node->NextEq;

        if ( !Node->NextEq )
        {
            //
            // This node is last in the chain of the Eq.
            //
            if ( Node->Parent == PrevEq )
            {
                //
                // No more in the chain of the Eq.
                //
                Node->Parent->LastEq = NULL;
            }
            else
            {
                Node->Parent->LastEq = PrevEq;
            }
#ifdef _DEBUG_REMOVE
            wcscpy( Mess, L"   Delete from the end of the chain of the Eq." );
#endif
        }
#ifdef _DEBUG_REMOVE
        else
        {
            wcscpy( Mess, L"   Delete from the midle of the chain of the Eq." );
        }
#endif

        Tree->NodeCnt.QuadPart--;

#ifdef _DEBUG_REMOVE
        swprintf( Mess+wcslen(Mess), L" NodeCnt %I64d", Tree->NodeCnt.QuadPart );
        OutMessage( Mess );
#endif

        return;
    }


    if ( Node->NextEq )
    {
        PBTR_NODE   NextEq = Node->NextEq;

#ifdef _DEBUG_REMOVE
        PrintNode( Tree->Root, Node );
#endif
        //
        // This node is the first in the chain of the Eq.
        // Move all links onto NextEq.
        //
        NextEq->Left   = Node->Left;
        if ( Node->Left )
        {
            Node->Left->Parent = NextEq;
        }

        NextEq->Right  = Node->Right;
        if ( Node->Right )
        {
            Node->Right->Parent = NextEq;
        }

        NextEq->Parent = Node->Parent;
        if ( Node->Parent )
        {
            if ( Node->Parent->Left == Node )
            {
                Node->Parent->Left = NextEq;
            }
            else
            {
                Node->Parent->Right = NextEq;
            }
        }
        else // if ( !Node->Parent )
        {
            Tree->Root = NextEq;
        }

        NextEq->LastEq = Node->LastEq;
        if ( Node->LastEq == NextEq )
        {
            NextEq->LastEq = NULL;
        }

        while ( NextEq->NextEq )
        {
            NextEq = NextEq->NextEq;
            NextEq->Parent = Node->NextEq;
        }

        Tree->NodeCnt.QuadPart--;

#ifdef _DEBUG_REMOVE
        swprintf( Mess, L"   Delete first in the chain of the Eq. NodeCnt %I64d",
                Tree->NodeCnt.QuadPart );
        OutMessage( Mess );
        PrintNode( Tree->Root, Node->NextEq );
#endif

        return;
    }

    //
    // Else this node do not participate in Eq chain
    //
    PBTR_NODE   x;
    PBTR_NODE   y;

    if ( !Node->Left || !Node->Right )
    {
        y = Node;
    }
    else
    {
        y = Node->Right;
        while ( y->Left ) 
        {
            y = y->Left;
        }
    }

    if ( y->Left )
    {
        x = y->Left;
    }
    else
    {
        x = y->Right;
    }

    if ( x )
    {
        x->Parent = y->Parent;
    }

    if ( y->Parent )
    {
        if ( y == y->Parent->Left )
        {
            y->Parent->Left = x;
        }
        else
        {
            y->Parent->Right = x;
        }
    }
    else 
    {
        Tree->Root = x;
    }

    if ( y != Node )
    {
        y->Left = Node->Left;
        if ( y->Left )
        {
            y->Left->Parent = y;
        }

        y->Right = Node->Right;
        if ( y->Right )
        {
            y->Right->Parent = y;
        }

        y->Parent = Node->Parent;
        if ( Node->Parent )
        {
            if ( Node == Node->Parent->Left )
            {
                Node->Parent->Left = y;
            }
            else
            {
                Node->Parent->Right = y;
            }
        }
        else
        {
            Tree->Root = y;
        }
    }

    Tree->NodeCnt.QuadPart--;

#ifdef _DEBUG_REMOVE
        swprintf( Mess, L"   Delete node. NodeCnt %I64d",
                Tree->NodeCnt.QuadPart );
        OutMessage( Mess );
#endif


    return;

} // end of RemoveNode


void
FreeNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node )
{
    if ( !Node ) 
    {
        return;
    }

    if ( Node->Body )
    {
        if ( Tree->Methods.FreeBody )
        {
            Tree->Methods.FreeBody( Node->Body );
        }
        else 
        {
            free( Node->Body );
        }
    }

    free( Node );


    return;

} // end of FreeNode


void
DeleteNode(
    PBTR_TREE       Tree,
    PBTR_NODE       Node )
{

    RemoveNode( Tree, Node );
    FreeNode( Tree, Node );

} // DeleteNode


PBTR_NODE
FindLeaf(
    PBTR_TREE       Tree,
    BOOLEAN         FindFirst,
    BOOLEAN         LeftToRight )
{
    PBTR_NODE       Current = NULL, Parent;
    BOOLEAN         StopOnLeaf = FindFirst;


    if ( Tree->Root && (Tree->Current || FindFirst) )
    {
        if ( FindFirst )
        {
            Current = Tree->Root;
        }
        else
        {
            Current = Tree->Current;
            if ( Current->NextEq )
            {
                Current = Current->NextEq;
                Tree->Current = Current;
                return  Current;
            }
            else if ( Current->Parent && Current->Parent->LastEq == Current ) // NextEq 
            {
                Current = Current->Parent;
            }
        }

        if ( LeftToRight )
        {
            do
            {
                if ( Current->Left )
                {
                    Current = Current->Left;
                }
                else if ( Current->Right )
                {
                    Current = Current->Right;
                }
                else
                {
                    if ( StopOnLeaf )
                    {
                        break;
                    }
                    else
                    {
                        Parent = Current->Parent;
                        if ( !Parent || !Parent->Right || Parent->Right == Current )
                        {
                            Current = Parent;
                            break;
                        }
                        else
                        {
                            Current = Parent->Right;
                            StopOnLeaf = TRUE;
                        }
                    }
                }
            } while( TRUE );
        }
        else
        {
            do
            {
                if ( Current->Right )
                {
                    Current = Current->Right;
                }
                else if ( Current->Left )
                {
                    Current = Current->Left;
                }
                else
                {
                    if ( StopOnLeaf )
                    {
                        break;
                    }
                    else
                    {
                        Parent = Current->Parent;
                        if ( !Parent || !Parent->Left || Parent->Left == Current )
                        {
                            Current = Parent;
                            break;
                        }
                        else
                        {
                            Current = Parent->Left;
                            StopOnLeaf = TRUE;
                        }
                    }
                }
            } while( TRUE );
        }
    }

    Tree->Current = Current;

    return  Current;

} // end of FindNextLeaf


void
RemoveLeaf(
    PBTR_TREE       Tree,
    PBTR_NODE       Node )
{
    if ( !Node ) 
    {
        return;
    }

    if ( Node->NextEq )
    {
        if ( Node->Parent && Node->Parent->NextEq == Node )
        {
            Node->Parent->NextEq = Node->NextEq;
        }
    }
    else if ( Node->Parent )
    {
        if ( Node->Parent->Left == Node )
        {
            Node->Parent->Left = NULL;
        }
        else if ( Node->Parent->Right == Node )
        {
            Node->Parent->Right = NULL;
        }
        else if ( Node->Parent->NextEq == Node )
        {
            Node->Parent->NextEq = NULL;
            Node->Parent->LastEq = NULL;
        }
    }

    Tree->NodeCnt.QuadPart--;

    return;

} // end of RemoveLeaf



void
DeleteLeaf(
    PBTR_TREE       Tree,
    PBTR_NODE       Node )
{

#ifdef _DEBUG
    //swprintf( Mess, L"Node %08I64d: %p", Cnt.QuadPart, Node );
    //OutMessage( Mess );
    //if ( Tree->Methods.PrintBody )
    //{
    //    Tree->Methods.PrintBody( Node->Body );
    //}
#endif

    RemoveLeaf( Tree, Node );
    FreeNode( Tree, Node );

} // end of DeleteLeaf

//
// Delete all tree nodes.
//
void
DeleteTree(
    PBTR_TREE   Tree )
{
	PBTR_NODE	    Node, NextNode, EqNode;
#ifdef _DEBUG
    WCHAR           Mess[MAX_PATH];
    LARGE_INTEGER   Cnt = { 0 }, BakCnt;
    BakCnt = Tree->NodeCnt;
#endif


    if ( !Tree )
    {
        return;
    }

    Node = FindLeaf( Tree, TRUE, LEFT_TO_RIGHT );
    while ( Node )
    {
        while ( Node->NextEq )
        {
            EqNode = Node->NextEq;
            DeleteLeaf( Tree, EqNode );

#ifdef _DEBUG
        Cnt.QuadPart++;
#endif
        }

        NextNode = FindLeaf( Tree, FALSE, LEFT_TO_RIGHT );
        DeleteLeaf( Tree, Node );

#ifdef _DEBUG
        Cnt.QuadPart++;
#endif

        Node = NextNode;
    }

#ifdef _DEBUG
    swprintf( Mess, L"Delete_left %I64d node from total %I64d. %I64d remained\n",
                    Cnt.QuadPart, BakCnt.QuadPart, Tree->NodeCnt.QuadPart );
    OutMessage( Mess );
#endif

    if ( (Tree->Flags & TREE_INITIALIZED) )
    {
        DeleteCriticalSection( &(Tree->TreeSync) );
    }

    memset( Tree, 0, sizeof( BTR_TREE ) );

} // end of DeleteTree


//
// Release all tree nodes but do not delete them.
//
void
ReleaseTree(
    PBTR_TREE   Tree )
{
	PBTR_NODE	    Node, NextNode, EqNode;
#ifdef _DEBUG
    WCHAR           Mess[MAX_PATH];
    LARGE_INTEGER   Cnt = { 0 }, BakCnt;
    BakCnt = Tree->NodeCnt;
#endif


    if ( !Tree )
    {
        return;
    }

    Node = FindLeaf( Tree, TRUE, LEFT_TO_RIGHT );
    while ( Node )
    {
        while ( Node->NextEq )
        {
            EqNode = Node->NextEq;
            RemoveLeaf( Tree, EqNode );

#ifdef _DEBUG
        Cnt.QuadPart++;
#endif
        }

        NextNode = FindLeaf( Tree, FALSE, LEFT_TO_RIGHT );
        RemoveLeaf( Tree, Node );

#ifdef _DEBUG
        Cnt.QuadPart++;
#endif

        Node = NextNode;
    }

#ifdef _DEBUG
    swprintf( Mess, L"Release_left %I64d node from total %I64d. %I64d remained\n",
                    Cnt.QuadPart, BakCnt.QuadPart, Tree->NodeCnt.QuadPart );
    OutMessage( Mess );
#endif

    if ( (Tree->Flags & TREE_INITIALIZED) )
    {
        DeleteCriticalSection( &(Tree->TreeSync) );
    }

    memset( Tree, 0, sizeof( BTR_TREE ) );

} // end of ReleaseTree


void
InitTree(
    PBTR_TREE           Tree,
    PBTR_TREE_METHODS   Methods,
    BTR_TREE_SORT_MODE  SortMode )
{

    if ( !Tree )
    {
        return;
    }

    DeleteTree( Tree );

    Tree->Methods.FreeBody      = Methods->FreeBody;
    Tree->Methods.CompareItems  = Methods->CompareItems;
    Tree->Methods.CopyBody      = Methods->CopyBody;
    Tree->Methods.PrintBody     = Methods->PrintBody;

    Tree->SortMode              = SortMode;

    InitializeCriticalSection( &(Tree->TreeSync) );
    Tree->Flags = TREE_INITIALIZED;

} // InitTree




