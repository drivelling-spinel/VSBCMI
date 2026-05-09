@ECHO OFF
SET EXEFILE=SSF.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Fixes issues with second joystick usage in two player games
PAUSE
ECHO Patching...

FPATCH -s837D1800750CC745FC01000000E93A01 -p+9 -r10 %EXEFILE% 
FPATCH -s837D1800750CC745FC10000000E93A01 -p+1b -r20 %EXEFILE% 
FPATCH -s837D1800750CC745FC10000000E93A01 -p+2d -r40 %EXEFILE% 
FPATCH -s837D1800750CC745FC10000000E93A01 -p+3f -r80 %EXEFILE% 
GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

