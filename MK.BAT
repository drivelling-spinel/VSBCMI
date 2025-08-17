@echo off
if not exist error.log goto noerr
del error.log
:noerr
make NAME=vsbhda REDIR=1 -f djgpp.mak
