#name: PE-COFF SizeOfImage
#ld: -T image_size.t
#objdump: -p
#target: *-*-mingw32 *-*-cygwin *-*-msys

.*:     file format .*
#...
SizeOfImage		00004000
#...
