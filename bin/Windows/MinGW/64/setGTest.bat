@echo off
rem Google Test MinGW 64-Bit Setup

rem Put a custom version first in your PATH to adapt to your system

set GTEST_INC=C:\gtest\include
set GTEST_BIN=C:\gtest\bin\MinGW\64
set Path=%Path%;%GTEST_BIN%
