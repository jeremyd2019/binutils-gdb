#as:
#source: array-char-conflicting-1.c
#source: array-char-conflicting-2.c
#objdump: --ctf
#cc: -fPIC
#ld: -shared --ctf-variables --hash-style=sysv
#name: Arrays (conflicted)

.*: +file format .*

Contents of type section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 5 \(CTF_VERSION_4\)
#...
  Variables:
    digits_names -> .* \(kind 4\) char \*\[10\] .*
#...
  Header:
#...
    Parent name: .ctf
#...
  Variables:
    digits_names -> .* \(kind 4\) char \*\[9\] .*
#...
