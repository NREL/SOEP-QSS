@echo off
:: Google Test Intel C++ 64-Bit Setup

:: Put a custom version first in your PATH to adapt to your system

set GTEST_INC=C:\gtest\include
set GTEST_BIN=C:\gtest\bin\IC\64\MDd
set Path=%Path%;%GTEST_BIN%
