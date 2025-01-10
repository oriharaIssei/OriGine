/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2022, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
/** @file vector2.h
 *  @brief 2D vector structure, including operators when compiling in C++
 */
#pragma once
#ifndef AI_VECTOR2D_H_INC
#define AI_VECTOR2D_H_INC

#ifdef __GNUC__
#   pragma GCC system_header
#endif

#ifdef __cplusplus
#   include <cmath>
#else
#   include <math.h>
#endif

#include "defs.h"

// ----------------------------------------------------------------------------------
/** Represents a two-dimensional vector.
 */

#ifdef __cplusplus
template <typename TReal>
class aiVec2ft {
public:
    aiVec2ft () : x(), y() {}
    aiVec2ft (TReal _x, TReal _y) : x(_x), y(_y) {}
    explicit aiVec2ft (TReal _xyz) : x(_xyz), y(_xyz) {}
    aiVec2ft (const aiVec2ft& o) = default;

    void Set( TReal pX, TReal pY);
    TReal SquareLength() const ;
    TReal Length() const ;
    aiVec2ft& Normalize();

    const aiVec2ft& operator += (const aiVec2ft& o);
    const aiVec2ft& operator -= (const aiVec2ft& o);
    const aiVec2ft& operator *= (TReal f);
    const aiVec2ft& operator /= (TReal f);

    TReal operator[](unsigned int i) const;

    bool operator== (const aiVec2ft& other) const;
    bool operator!= (const aiVec2ft& other) const;

    bool Equal(const aiVec2ft &other, TReal epsilon = ai_epsilon) const;

    aiVec2ft& operator= (TReal f);
    const aiVec2ft SymMul(const aiVec2ft& o);

    template <typename TOther>
    operator aiVec2ft<TOther> () const;

    TReal x, y;
};

typedef aiVec2ft<ai_real> aiVec2fD;

#else

struct aiVec2fD {
    ai_real x, y;
};

#endif // __cplusplus

#endif // AI_VECTOR2D_H_INC
