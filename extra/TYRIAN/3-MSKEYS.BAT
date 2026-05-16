@ECHO OFF
SET EXEFILE=FILE0001.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Unbinds keys [M] and [S] from sound and music toggle
ECHO    - Allows using [M] and [S] in keyboard configuration menu
PAUSE
ECHO Patching...

FPATCH -s1F743480 -p-4 -r90909090909090 -s32742D80 -p-4 -r90909090909090 %EXEFILE% 
FPATCH -s00743CC6 -p-4 -rEB41909090 -s007445C6 -p-4 -rEB4A909090 %EXEFILE% 
GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

