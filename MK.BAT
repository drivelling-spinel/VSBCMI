@echo off
if not exist error.log goto noerr
del error.log
:noerr
make NAME=VSBCMI REDIR=1 -f djgpp.mak
