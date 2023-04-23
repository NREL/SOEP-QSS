@echo off
setlocal

if (%1) == () (
  echo Error: No argument passed to hdr.bat
  goto Exit
)

set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=%1
mkdir %FMIL_INS_DIR%\include\src >nul 2>&1
mkdir %FMIL_INS_DIR%\include\src\FMI >nul 2>&1
mkdir %FMIL_INS_DIR%\include\src\FMI2 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\CAPI\include\FMI2\fmi2_capi.h %FMIL_INS_DIR%\include\FMI2 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\CAPI\src\FMI2\fmi2_capi_impl.h %FMIL_INS_DIR%\include\src\FMI2 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\Import\src\FMI\fmi_import_context_impl.h %FMIL_INS_DIR%\include\FMI >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\Import\src\FMI2\fmi2_import_impl.h %FMIL_INS_DIR%\include\FMI2 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\Util\src\FMI\fmi_util_options_impl.h %FMIL_INS_DIR%\include\src\FMI >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\XML\include\FMI\*.h %FMIL_INS_DIR%\include\FMI >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\XML\include\FMI1\*.h %FMIL_INS_DIR%\include\FMI1 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\src\XML\include\FMI2\*.h %FMIL_INS_DIR%\include\FMI2 >nul 2>&1
copy /Y %FMIL_SRC_DIR%\ThirdParty\Expat\expat-2.4.8\lib\expat*.h %FMIL_INS_DIR%\include >nul 2>&1

endlocal

:Exit
