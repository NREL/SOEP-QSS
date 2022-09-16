@echo off
:: Run unit tests with or without filtering
if "%1" == "" (
  QSS.unit.exe
) else (
  QSS.unit.exe --gtest_filter="%1"
)
