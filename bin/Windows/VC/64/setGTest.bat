@echo off
rem Google Test Visual C++ 64-Bit Setup

rem Put a custom version first in your PATH to adapt to your system

set GTEST_INC=C:\gtest\include
set GTEST_BIN=C:\gtest\bin\VC\64\MDd
set Path=%Path%;%GTEST_BIN%
