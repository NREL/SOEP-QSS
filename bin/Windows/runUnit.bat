@echo off
:: Run unit tests with a filter
QSS.unit.exe --gtest_filter="%1"
