@echo off
:: Google Test GCC 64-Bit Setup

:: Put a custom version first in your PATH to adapt to your system

set GTEST_INC=C:\gtest\include
set GTEST_BIN=C:\gtest\bin\GCC\64
set Path=%Path%;%GTEST_BIN%
