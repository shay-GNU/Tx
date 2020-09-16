@echo off
set CC=gcc

%CC% txclib.c -shared -DTX_DLL -O2 -o txclib.dll
%CC% txlib.c -shared -DTX_DLL -O2 -o txlib.dll
%CC% tx.c txlib.dll txclib.dll  -DTX_DLL -O2 -o tx
%CC% txc.c txclib.dll txlib.dll -DTX_DLL -O2 -o txc