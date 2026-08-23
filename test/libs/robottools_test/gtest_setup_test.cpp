/***************************************************************************

    file                 : gtest_setup_test.cpp
    created              : Mon Nov 11 20:00:00 CET 2025
    copyright            : (C) 2025-2025 by Bernhard Wymann
    email                : berniw@bluewin.ch

 ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

/**
    Verify gtest setup
*/

#include <gtest/gtest.h>

TEST(GTestSetupTest, AlwaysPasses) {
    EXPECT_EQ(1, 1);
}