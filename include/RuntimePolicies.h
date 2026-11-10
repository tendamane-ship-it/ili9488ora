#ifndef RUNTIME_POLICIES_H
#define RUNTIME_POLICIES_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace runtime_policies
{
constexpr std::size_t RAM_PROTECTION_LOW_BYTES = 100UL * 1024UL;
constexpr std::size_t RAM_PROTECTION_RECOVERY_BYTES = 130UL * 1024UL;

class RamProtection
{
public:
    bool update(std::size_t freeHeapBytes)
    {
        if (!active_ && freeHeapBytes < RAM_PROTECTION_LOW_BYTES)
            active_ = true;
        else if (active_ &&
                 freeHeapBytes > RAM_PROTECTION_RECOVERY_BYTES)
            active_ = false;

        return active_;
    }

    bool active() const
    {
        return active_;
    }

private:
    bool active_ = false;
};

template <std::size_t Capacity>
class ResetHistory
{
    static_assert(Capacity > 0, "Reset history capacity must be positive");

public:
    void clear()
    {
        entries_.fill(0);
        nextIndex_ = 0;
        length_ = 0;
    }

    bool restore(
        const uint8_t* entries,
        std::size_t nextIndex,
        std::size_t length
    )
    {
        if (entries == nullptr || nextIndex >= Capacity || length > Capacity)
            return false;

        for (std::size_t i = 0; i < Capacity; ++i)
            entries_[i] = entries[i];

        nextIndex_ = nextIndex;
        length_ = length;
        return true;
    }

    void push(uint8_t reason)
    {
        entries_[nextIndex_] = reason;
        nextIndex_ = (nextIndex_ + 1) % Capacity;
        if (length_ < Capacity)
            ++length_;
    }

    std::size_t size() const
    {
        return length_;
    }

    std::size_t nextIndex() const
    {
        return nextIndex_;
    }

    const uint8_t* data() const
    {
        return entries_.data();
    }

    bool atOldestFirst(std::size_t index, uint8_t& reason) const
    {
        if (index >= length_)
            return false;

        const std::size_t oldestIndex =
            (nextIndex_ + Capacity - length_) % Capacity;
        reason = entries_[(oldestIndex + index) % Capacity];
        return true;
    }

private:
    std::array<uint8_t, Capacity> entries_{};
    std::size_t nextIndex_ = 0;
    std::size_t length_ = 0;
};
}

#endif
