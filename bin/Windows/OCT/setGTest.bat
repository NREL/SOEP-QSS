@echo off
:: Google Test OCT MinGW Setup

:: Put a custom version first in your PATH to adapt to your system

set GTEST_BIN=C:\gtest\bin\OCT
set GTEST_INC=C:\gtest\include
set Path=%Path%;%GTEST_BIN%
