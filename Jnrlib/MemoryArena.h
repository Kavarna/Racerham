#pragma once

#include "BasicTypes.h"
#include "Jnrlib/Check.h"

#include <algorithm>
#include <array>
#include <type_traits>
#include <unordered_map>
#include <vector>

template <u32 Size, typename T = unsigned char> class MemoryArena
{
private:
    struct Block
    {
        u32 offset;
        u32 size;

        Block(u32 offset, u32 size) : offset(offset), size(size)
        {
        }

        friend bool operator<(Block const lhs, Block const rhs)
        {
            return lhs.offset < rhs.offset;
        }
    };

public:
    MemoryArena()
    {
        mFreeBlocks.emplace_back(0, (u32)Size * (u32)sizeof(T));
    };
    ~MemoryArena() = default;

public:
    template <typename... Args> T *Allocate(u32 count, Args &&...args)
    {
        CHECK(count <= Size, nullptr, "Size is bigger than maximum size");
        CHECK(mFreeBlocks.size() != 0, nullptr,
              "No more free blocks available");

        T *returnValue = nullptr;
        for (size_t i = 0; i < mFreeBlocks.size(); ++i)
        {
            if (mFreeBlocks[i].size >= count * sizeof(T))
            {
                Block usedBlock(mFreeBlocks[i].offset, count * sizeof(T));
                mFreeBlocks[i].offset += count * sizeof(T);
                mFreeBlocks[i].size -= count * sizeof(T);
                if (mFreeBlocks[i].size == 0)
                {
                    mFreeBlocks.erase(mFreeBlocks.begin() + i);
                }

                returnValue = reinterpret_cast<T *>(&mData[usedBlock.offset]);
                mUsedBlocks.insert({returnValue, usedBlock});
                break;
            }
        }

        CHECK(returnValue != nullptr, nullptr, "Unable to find a free block");

        if constexpr (std::is_default_constructible_v<T>)
        {
            new (returnValue) T;
        }
        else
        {
            new (returnValue) T(std::forward<Args>(args)...);
        }
        return returnValue;
    }

    void Free(T *data)
    {
        ThrowIfFailed(data != nullptr, "Can't free nullptr");

        auto it = mUsedBlocks.find(data);
        ThrowIfFailed(it != mUsedBlocks.end(),
                      "Can't free block unallocated by this memory arena");

        Block currentBlock = it->second;

        Block *previousFreeBlock = nullptr;
        Block *nextFreeBlock = nullptr;
        for (size_t i = 0; i < mFreeBlocks.size(); ++i)
        {
            if (mFreeBlocks[i].offset <= currentBlock.offset)
            {
                ThrowIfFailed(mFreeBlocks[i].offset + mFreeBlocks[i].size <=
                                  currentBlock.offset,
                              "A free block can't overlap a used block");
                if (mFreeBlocks[i].offset + mFreeBlocks[i].size ==
                    currentBlock.offset)
                {
                    /* We only care about the previous block only if it's we
                     * could merge it */
                    previousFreeBlock = &mFreeBlocks[i];
                }
            }
            if (mFreeBlocks[i].offset > currentBlock.offset)
            {
                ThrowIfFailed(currentBlock.offset + currentBlock.size <=
                                  mFreeBlocks[i].offset,
                              "A free block can't overlap a used block");
                if (currentBlock.offset + currentBlock.size ==
                    mFreeBlocks[i].offset)
                {
                    /* We only care about the next block only if we could merge
                     * it */
                    nextFreeBlock = &mFreeBlocks[i];
                }
                break;
            }
        }

        mUsedBlocks.erase(it);
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            data->~T();
        }

        /* Merge the existing blocks */
        if (previousFreeBlock && nextFreeBlock)
        {
            previousFreeBlock->size += currentBlock.size + nextFreeBlock->size;

            u32 nextFreeBlockIndex = nextFreeBlock - mFreeBlocks.data();
            mFreeBlocks.erase(mFreeBlocks.begin() + nextFreeBlockIndex);
        }
        else if (previousFreeBlock)
        {
            previousFreeBlock->size += currentBlock.size;
        }
        else if (nextFreeBlock)
        {
            nextFreeBlock->size += currentBlock.size;
            nextFreeBlock->offset -= currentBlock.size;
        }
        else /* previousFreeBlock == nullptr && nextFreeBlock == nullptr */
        {
            ThrowIfFailed(
                previousFreeBlock == nullptr,
                "Previous free block expected to be nullptr but it is not");
            ThrowIfFailed(
                nextFreeBlock == nullptr,
                "Next free block expected to be nullptr but it is not");

            /* TODO JNR: Replace this with insert at position */
            mFreeBlocks.push_back(std::move(currentBlock));
            std::sort(mFreeBlocks.begin(), mFreeBlocks.end());
        }
    }

private:
    std::array<unsigned char, Size * sizeof(T)> mData = {};

    std::vector<Block> mFreeBlocks;
    std::unordered_map<T *, Block> mUsedBlocks;
};