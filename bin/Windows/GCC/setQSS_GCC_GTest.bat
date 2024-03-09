@echo off
:: Google Test GCC Setup

:: Put a custom version earlier in your PATH to adapt to your system

set GTEST_BIN=C:\gtest\bin\GCC
set GTEST_INC=C:\gtest\include
set Path=%Path%;%GTEST_BIN%
