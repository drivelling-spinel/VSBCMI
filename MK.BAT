@echo off
hdpmi32i -r
if not exist error.log goto noerr
del error.log
:noerr
make NAME=VSBCMIdj REDIR=1 -f djgpp.mak
hdpmi32i -u
