#include "Tags/GASExtGameplayTagStack.h"

//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

FString FGASExtGameplayTagStack::GetDebugString() const
{
    return FString::Printf( TEXT( "%sx%d" ), *Tag.ToString(), StackCount );
}

//////////////////////////////////////////////////////////////////////
// FGASExtGameplayTagStackContainer

void FGASExtGameplayTagStackContainer::AddStack( FGameplayTag Tag, int32 StackCount )
{
    if ( !Tag.IsValid() )
    {
        FFrame::KismetExecutionMessage( TEXT( "An invalid tag was passed to AddStack" ), ELogVerbosity::Warning );
        return;
    }

    if ( StackCount > 0 )
    {
        for ( FGASExtGameplayTagStack & Stack : Stacks )
        {
            if ( Stack.Tag == Tag )
            {
                const int32 NewCount = Stack.StackCount + StackCount;
                Stack.StackCount = NewCount;
                TagToCountMap[ Tag ] = NewCount;
                MarkItemDirty( Stack );
                return;
            }
        }

        FGASExtGameplayTagStack & NewStack = Stacks.Emplace_GetRef( Tag, StackCount );
        MarkItemDirty( NewStack );
        TagToCountMap.Add( Tag, StackCount );
    }
}

void FGASExtGameplayTagStackContainer::RemoveStack( FGameplayTag Tag, int32 StackCount )
{
    if ( !Tag.IsValid() )
    {
        FFrame::KismetExecutionMessage( TEXT( "An invalid tag was passed to RemoveStack" ), ELogVerbosity::Warning );
        return;
    }

    //@TODO: Should we error if you try to remove a stack that doesn't exist or has a smaller count?
    if ( StackCount > 0 )
    {
        for ( auto It = Stacks.CreateIterator(); It; ++It )
        {
            FGASExtGameplayTagStack & Stack = *It;
            if ( Stack.Tag == Tag )
            {
                if ( Stack.StackCount <= StackCount )
                {
                    It.RemoveCurrent();
                    TagToCountMap.Remove( Tag );
                    MarkArrayDirty();
                }
                else
                {
                    const int32 NewCount = Stack.StackCount - StackCount;
                    Stack.StackCount = NewCount;
                    TagToCountMap[ Tag ] = NewCount;
                    MarkItemDirty( Stack );
                }
                return;
            }
        }
    }
}

void FGASExtGameplayTagStackContainer::PreReplicatedRemove( const TArrayView< int32 > RemovedIndices, int32 FinalSize )
{
    for ( int32 Index : RemovedIndices )
    {
        const FGameplayTag Tag = Stacks[ Index ].Tag;
        TagToCountMap.Remove( Tag );
    }
}

void FGASExtGameplayTagStackContainer::PostReplicatedAdd( const TArrayView< int32 > AddedIndices, int32 FinalSize )
{
    for ( int32 Index : AddedIndices )
    {
        const FGASExtGameplayTagStack & Stack = Stacks[ Index ];
        TagToCountMap.Add( Stack.Tag, Stack.StackCount );
    }
}

void FGASExtGameplayTagStackContainer::PostReplicatedChange( const TArrayView< int32 > ChangedIndices, int32 FinalSize )
{
    for ( int32 Index : ChangedIndices )
    {
        const FGASExtGameplayTagStack & Stack = Stacks[ Index ];
        TagToCountMap[ Stack.Tag ] = Stack.StackCount;
    }
}
