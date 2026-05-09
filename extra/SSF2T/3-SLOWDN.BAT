@ECHO OFF
SET EXEFILE=SF2TURBO.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE

IF Z==Z%1 GOTO DEFVAL
IF Z==Z%2 GOTO ONEB

SET SF=-%2-%1-
GOTO VALDONE

:ONEB
SET SF=-%1-00-
GOTO VALDONE

:DEFVAL
SET SF=-00-02-
GOTO VALDONE

:VALDONE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Joystick polling rate toned down for faster machines
ECHO This patch accepts slowdown factor in the following format:
ECHO    %0 12 34
ECHO    which translates to 1234 hexadecimal
ECHO    %0 12
ECHO    which translates to 0012 hexadecimal
ECHO    %0
ECHO    when run without arguments uses default value 0200 (hexadecimal)
PAUSE
ECHO Patching...

FPATCH -s558BEC6652665166536650FA -r60fa668b5c242466ba01026631c9ee6641 %EXEFILE%
FPATCH -s66ba01026631c9ee6641 -p+#10 -r6683f9ff740f51b9%SF%000040e2fd59ec84c375e9 %EXEFILE%
FPATCH -s66ba01026631c9ee6641 -p+#31 -r66890d04e83200fb6631c98a1d09e8320066416683f9ff740f51b9%SF%000040e2fd59ec84c3 %EXEFILE%
FPATCH -s66ba01026631c9ee6641 -p+#69 -r75e961c3 %EXEFILE%

GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

