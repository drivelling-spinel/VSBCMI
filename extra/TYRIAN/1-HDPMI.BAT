@ECHO OFF
SET EXEFILE=FILE0001.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Support for running with IOPL0 with HDPMI DPMI server 
PAUSE
ECHO Patching...

FPATCH -m -s5589E59CFAB036 -p+3 -r90 -s9DC9CB -rFB %EXEFILE% 
FPATCH -s60069CFA -p+2 -r9090 -s9D0761C9 -rFB %EXEFILE% 
FPATCH -m -sC39CFA -p+1 -r90 -s9DC3 -rFB %EXEFILE% 
FPATCH -m -sCD211F9D -p+3 -rFB %EXEFILE% 
FPATCH -m -s9CFAB8 -r90 %EXEFILE% 
FPATCH -m -sE1F79CFA8A -p+2 -r90 %EXEFILE% 
FPATCH -m -sE6219DB9 -p+2 -rFB %EXEFILE% 
FPATCH -m -s9CFAA0 -r90 %EXEFILE% 
FPATCH -m -s9CFA80FC -r90 %EXEFILE% 
FPATCH -m -s9CFAE8 -r90 -s9D5EEB -rFB %EXEFILE% 
FPATCH -m -s9CFA53 -r90 -s9D1FC3 -rFB %EXEFILE% 
FPATCH -m -s9D58A3 -rFB %EXEFILE% 
 


GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

