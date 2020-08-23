/* Copyright (C) 2020 Gleb Bezborodov - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * You should have received a copy of the MIT license with
 * this file. If not, please write to: bezborodoff.gleb@gmail.com, or visit : https://github.com/glensand/hope
 */

#include "small_object_allocator.h"
#include <algorithm>

namespace hope::memory {

    small_object_allocator& small_object_allocator::instance() {
        static small_object_allocator instance;
        return instance;
    }

    void small_object_allocator::deallocate(void* ptr, std::size_t size) noexcept {
        if (size > m_max_object_size) [[unlikely]] {
            delete[] static_cast<uint8_t*>(ptr);
        } else [[likely]] {
            if constexpr (!config::MemoryReductionMode)
            {
                m_allocator_list[size / config::PointerAlignment - 1].deallocate(ptr);
            }
            else
            {
                const auto alloc = find_allocator(size);
                assert(alloc);
                alloc->deallocate(ptr);
            }
        }
    }

    void* small_object_allocator::allocate(std::size_t size) noexcept{
        if(size > m_max_object_size) [[unlikely]] {
            try {
                return new uint8_t[size];
            }
            catch (const std::bad_alloc& ex) {
                assert(false);
                return nullptr;
            }
        }
        if constexpr(!config::MemoryReductionMode)
            return m_allocator_list[size / config::PointerAlignment - 1].allocate();
        if (m_allocator_list.size() == m_max_object_size / config::PointerAlignment) [[likely]]
            return m_allocator_list[size / config::PointerAlignment - 1].allocate();
        auto alloc = find_allocator(size);
        if (alloc == nullptr) [[unlikely]]
            alloc = create_allocator(size);
        assert(alloc != nullptr);
        return alloc->allocate();
    }

    fixed_allocator* small_object_allocator::find_allocator(std::size_t size) noexcept {
        const auto allocIt = std::find_if(std::begin(m_allocator_list), std::end(m_allocator_list),
            [=](const auto& allocator) {
                return allocator.block_size() == size;
            });
        return allocIt != std::end(m_allocator_list) ? &*allocIt : nullptr;
    }

    fixed_allocator* small_object_allocator::create_allocator(std::size_t size) noexcept {
        const auto forwardIt = std::find_if(std::begin(m_allocator_list), std::end(m_allocator_list),
            [=](const auto& allocator) {
                return allocator.block_size() > size;
            });
        const auto isEnd = forwardIt == std::end(m_allocator_list);
        return &*m_allocator_list.emplace(forwardIt, m_chunk_size, size);
    }

    void small_object_allocator::clear() {
        assert(config::TestMode);
        m_allocator_list.clear();
    }

    void small_object_allocator::initialize_allocators() {
        m_allocator_list.reserve(m_max_object_size / config::PointerAlignment);
        for (std::size_t i{ 0 }; i < m_max_object_size / config::PointerAlignment; ++i)
            m_allocator_list.emplace_back(m_chunk_size, (i + 1) * config::PointerAlignment);
    }

    small_object_allocator::small_object_allocator(){
        if constexpr (!config::MemoryReductionMode)
            initialize_allocators();
    }

    small_object_allocator::~small_object_allocator() {
        clear();
    }
}
