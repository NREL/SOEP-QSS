@echo off
rem Run unit tests with a filter
QSS.unit.exe --gtest_filter="%1"
