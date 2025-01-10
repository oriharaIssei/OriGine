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
/** @file vector3.h
 *  @brief 3D vector structure, including operators when compiling in C++
 */
#pragma once
#ifndef AI_VECTOR3D_H_INC
#define AI_VECTOR3D_H_INC

#ifdef __GNUC__
#   pragma GCC system_header
#endif

#ifdef __cplusplus
#   include <cmath>
#else
#   include <math.h>
#endif

#include <assimp/defs.h>

#ifdef __cplusplus

template<typename TReal> class aiMatrix3x3t;
template<typename TReal> class aiMatrix4x4t;

// ---------------------------------------------------------------------------
/// @brief  Represents a three-dimensional vector.
// ---------------------------------------------------------------------------
template <typename TReal>
class aiVec3ft {
public:
    /// @brief  The default class constructor.
    aiVec3ft() AI_NO_EXCEPT : x(), y(), z() {}

    /// @brief  The class constructor with the components.
    /// @param  _x  The x-component for the vector.
    /// @param  _y  The y-component for the vector.
    /// @param  _z  The z-component for the vector.
    aiVec3ft(TReal _x, TReal _y, TReal _z) : x(_x), y(_y), z(_z) {}

    /// @brief  The class constructor with a default value.
    /// @param  _xyz  The value for x, y and z.
    explicit aiVec3ft (TReal _xyz ) : x(_xyz), y(_xyz), z(_xyz) {}

    /// @brief  The copy constructor.
    /// @param  o The instance to copy from.
    aiVec3ft( const aiVec3ft& o ) = default;

    /// @brief  combined operators
    /// @brief  The copy constructor.
    const aiVec3ft& operator += (const aiVec3ft& o);

    /// @brief  The copy constructor.
    const aiVec3ft& operator -= (const aiVec3ft& o);

    /// @brief  The copy constructor.
    const aiVec3ft& operator *= (TReal f);

    /// @brief  The copy constructor.
    const aiVec3ft& operator /= (TReal f);

    /// @brief  Transform vector by matrix
    aiVec3ft& operator *= (const aiMatrix3x3t<TReal>& mat);
    aiVec3ft& operator *= (const aiMatrix4x4t<TReal>& mat);

    /// @brief  access a single element, const.
    TReal operator[](unsigned int i) const;

    /// @brief  access a single element, non-const.
    TReal& operator[](unsigned int i);

    // comparison
    bool operator== (const aiVec3ft& other) const;
    bool operator!= (const aiVec3ft& other) const;
    bool operator < (const aiVec3ft& other) const;

    /// @brief
    bool Equal(const aiVec3ft &other, TReal epsilon = ai_epsilon) const;

    template <typename TOther>
    operator aiVec3ft<TOther> () const;

    /** @brief Set the components of a vector
     *  @param pX X component
     *  @param pY Y component
     *  @param pZ Z component  */
    void Set( TReal pX, TReal pY, TReal pZ);

    /** @brief Get the squared length of the vector
     *  @return Square length */
    TReal SquareLength() const;

    /** @brief Get the length of the vector
     *  @return length */
    TReal Length() const;


    /** @brief Normalize the vector */
    aiVec3ft& Normalize();

    /** @brief Normalize the vector with extra check for zero vectors */
    aiVec3ft& NormalizeSafe();

    /** @brief Componentwise multiplication of two vectors
     *
     *  Note that vec*vec yields the dot product.
     *  @param o Second factor */
    const aiVec3ft SymMul(const aiVec3ft& o);

    TReal x, y, z;
};


typedef aiVec3ft<ai_real> aiVec3fD;

#else

struct aiVec3fD {
    ai_real x, y, z;
};

#endif // __cplusplus

#ifdef __cplusplus

#endif // __cplusplus

#endif // AI_VECTOR3D_H_INC
