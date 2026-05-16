@ECHO OFF
SET EXEFILE=FILE0001.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Support for running with IOPL0 with HDPMI DPMI server. 
ECHO      This is a mutually exclusive version with 1-HDPMI.BAT.
ECHO      Unlike 1-HDPMI.BAT it modifies .EXE in a gentler way
ECHO      leaving some of the game screens broken with HDPMI.
PAUSE
ECHO Patching...

FPATCH -s5589E560069CFA -p+6 -r90 -s509CFAE8 -p+2 -r90  %EXEFILE% 
GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

