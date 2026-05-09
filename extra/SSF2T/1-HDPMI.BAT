@ECHO OFF
SET EXEFILE=SF2TURBO.EXE
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Support for HDPMI DPMI server added
PAUSE
ECHO Patching...

FPATCH -s501EFC -r60 -s1FB020E62066B80109CD3158CF -p+#11 -r61 %EXEFILE% 
GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

