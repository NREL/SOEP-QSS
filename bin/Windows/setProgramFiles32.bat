@echo off
:: Set the ProgramFiles32 Env Var

if defined ProgramFiles32 goto Exit
set ProgramFiles32=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set ProgramFiles32=%ProgramFiles(x86)%
:Exit
