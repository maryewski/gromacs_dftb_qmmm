/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2020, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \internal \file
 * \brief
 * This implements basic nblib utility tests
 *
 * \author Victor Holanda <victor.holanda@cscs.ch>
 * \author Joe Jordan <ejjordan@kth.se>
 * \author Prashanth Kanduri <kanduri@cscs.ch>
 * \author Sebastian Keller <keller@cscs.ch>
 */

#include "nblib/tests/testhelpers.h"
#include "nblib/util/internal.h"
#include "nblib/util/user.h"

namespace nblib
{

TEST(NblibInternalUtils, FindIndexTuple1)
{
    using TupleType = std::tuple<float>;

    constexpr int floatIndex = FindIndex<float, TupleType>{};

    constexpr int outOfRange = FindIndex<unsigned, TupleType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(1, outOfRange);
}

TEST(NblibInternalUtils, FindIndexTuple2)
{
    using TupleType = std::tuple<float, int>;

    constexpr int floatIndex = FindIndex<float, TupleType>{};
    constexpr int intIndex   = FindIndex<int, TupleType>{};

    constexpr int outOfRange = FindIndex<unsigned, TupleType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(1, intIndex);
    EXPECT_EQ(2, outOfRange);
}

TEST(NblibInternalUtils, FindIndexTypeList1)
{
    using ListType = TypeList<float>;

    constexpr int floatIndex = FindIndex<float, ListType>{};

    constexpr int outOfRange = FindIndex<unsigned, ListType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(1, outOfRange);
}

TEST(NblibInternalUtils, FindIndexTypeList2)
{
    using ListType = TypeList<float, int>;

    constexpr int floatIndex = FindIndex<float, ListType>{};
    constexpr int intIndex   = FindIndex<int, ListType>{};

    constexpr int outOfRange = FindIndex<unsigned, ListType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(1, intIndex);
    EXPECT_EQ(2, outOfRange);
}


TEST(NblibInternalUtils, Contains)
{
    using ListType = TypeList<float, int>;

    constexpr bool hasFloat = Contains<float, ListType>{};
    constexpr bool hasInt   = Contains<int, ListType>{};
    constexpr bool hasUint  = Contains<unsigned, ListType>{};

    EXPECT_TRUE(hasFloat);
    EXPECT_TRUE(hasInt);
    EXPECT_FALSE(hasUint);
}

TEST(NblibInternalUtils, FindIndexTupleRepeated)
{
    using TupleType = std::tuple<float, float, int>;

    constexpr int floatIndex = FindIndex<float, TupleType>{};

    constexpr int intIndex = FindIndex<int, TupleType>{};

    constexpr int outOfRange = FindIndex<unsigned, TupleType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(2, intIndex);
    EXPECT_EQ(3, outOfRange);
}

TEST(NblibInternalUtils, FindIndexTypeListRepeated)
{
    using TupleType = TypeList<float, float, int>;

    constexpr int floatIndex = FindIndex<float, TupleType>{};

    constexpr int intIndex = FindIndex<int, TupleType>{};

    constexpr int outOfRange = FindIndex<unsigned, TupleType>{};

    EXPECT_EQ(0, floatIndex);
    EXPECT_EQ(2, intIndex);
    EXPECT_EQ(3, outOfRange);
}


} // namespace nblib