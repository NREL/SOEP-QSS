:: Run unit tests with or without filtering
@echo off
:: Use : separator between filter patterns
if "%1" == "" (
  QSS.unit.exe
) else (
  QSS.unit.exe --gtest_filter="%1"
)
