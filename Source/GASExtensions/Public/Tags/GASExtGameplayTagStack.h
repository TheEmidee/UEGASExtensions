#pragma once

#include "GameplayTagContainer.h"

#include <CoreMinimal.h>

#include "GASExtGameplayTagStack.generated.h"

struct FGASExtGameplayTagStackContainer;

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayTagStack : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGASExtGameplayTagStack()
    {}

    FGASExtGameplayTagStack( FGameplayTag InTag, int32 InStackCount ) :
        Tag( InTag ),
        StackCount( InStackCount )
    {
    }

    FString GetDebugString() const;

private:
    friend FGASExtGameplayTagStackContainer;

    UPROPERTY()
    FGameplayTag Tag;

    UPROPERTY()
    int32 StackCount = 0;
};

USTRUCT( BlueprintType )
struct GASEXTENSIONS_API FGASExtGameplayTagStackContainer : public FFastArraySerializer
{
    GENERATED_BODY()

    FGASExtGameplayTagStackContainer()
    {
    }

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    void AddStack( FGameplayTag Tag, int32 StackCount );

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    void RemoveStack( FGameplayTag Tag, int32 StackCount );

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    int32 GetStackCount( FGameplayTag Tag ) const
    {
        return TagToCountMap.FindRef( Tag );
    }

    // Returns true if there is at least one stack of the specified tag
    bool ContainsTag( FGameplayTag Tag ) const
    {
        return TagToCountMap.Contains( Tag );
    }

    //~FFastArraySerializer contract
    void PreReplicatedRemove( const TArrayView< int32 > RemovedIndices, int32 FinalSize );
    void PostReplicatedAdd( const TArrayView< int32 > AddedIndices, int32 FinalSize );
    void PostReplicatedChange( const TArrayView< int32 > ChangedIndices, int32 FinalSize );
    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize( FNetDeltaSerializeInfo & DeltaParms )
    {
        return FFastArraySerializer::FastArrayDeltaSerialize< FGASExtGameplayTagStack, FGASExtGameplayTagStackContainer >( Stacks, DeltaParms, *this );
    }

private:
    // Replicated list of gameplay tag stacks
    UPROPERTY()
    TArray< FGASExtGameplayTagStack > Stacks;

    // Accelerated list of tag stacks for queries
    TMap< FGameplayTag, int32 > TagToCountMap;
};

template <>
struct TStructOpsTypeTraits< FGASExtGameplayTagStackContainer > : public TStructOpsTypeTraitsBase2< FGASExtGameplayTagStackContainer >
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};
