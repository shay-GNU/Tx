@echo off
set TX=..\..\src\tx.exe
set TXC=..\..\src\txc.exe
set NAME=temp.out

%TXC% temp.tx -o %NAME%
%TX% %NAME%
pause
del %NAME%