@ECHO OFF
SET EXEFILE=USBJSTIK.COM
IF NOT EXIST FPATCH.COM GOTO NOFPAT
IF NOT EXIST %EXEFILE% GOTO NOFILE
ECHO This patch will update %EXEFILE%. 
ECHO Please always keep backup copy of the original files!
ECHO To abort press Ctrl+C now.
ECHO What changes:
ECHO    - Accept joysticks with not fully correctly formed HID descriptors  
ECHO    - Handle negative minimum values / parameter readings, e.g. for axes 
PAUSE
ECHO Patching...

FPATCH -s538A1ED00DA0D20D80FB017411 -p+1e -rb3033c36743a3c377436b3023c3974303c9072043c937628b3013c3d72043c3e %EXEFILE%
FPATCH -s538A1ED00DA0D20D80FB017411 -p+3e -r761eB3059090909090909090EB12 %EXEFILE%

FPATCH -s50535152A1E00D -r60a1e00d8b1ee20d39d8722808e475019839d87c1f %EXEFILE%
FPATCH -s60a1e00d8b1e -p+15 -r8b0ee40d8b16e60d09c9751329ca740f89d929c139d17507a1e40d8b1ee60d894504895d06619090 %EXEFILE%

FPATCH -s535152E83500722BE89200 -p+b -r85c979019829c829ca89d3f7262f01f7f39090 %EXEFILE%

GOTO DONE

:NOFPAT
ECHO ERROR: Please put FPATCH.COM side by side with this batch file for it to work.
GOTO DONE

:NOFILE
ECHO ERROR: File to patch - %EXEFILE% - not found.
GOTO DONE

:DONE
ECHO Batch file created in 2026 by Ludicrous_peridot

