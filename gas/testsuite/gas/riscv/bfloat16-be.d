# source: bfloat16.s
# objdump: -sj .data
# as: -mbig-endian

.*:[   ]+file format .*

Contents of section \.data:
 0000 41403dfc 000042f7 8000c2f7 7fff7f80.*
 0010 ff807f7f ff7f0080 80800001 8001007f.*
 0020 807f3f80 bf804000 c000ffc1 ff81.*
