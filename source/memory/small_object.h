/* Copyright (C) 2020 Gleb Bezborodov - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * You should have received a copy of the MIT license with
 * this file. If not, please write to: bezborodoff.gleb@gmail.com, or visit : https://github.com/glensand/hope
 */

#pragma once

#include "small_object_allocator.h"

namespace hope::memory {

    /**
     * \brief thin useful wrapper of small_object_allocator
     * just inherit your class from this, and u get height quality boost of performance :)
     * take safe and fasten seat belts, the rocket starts now!
     */
    class small_object {
    public:
        virtual ~small_object() = default;

        static void* operator new(std::size_t size);
        static void operator delete(void* ptr, std::size_t size);
    };

}
