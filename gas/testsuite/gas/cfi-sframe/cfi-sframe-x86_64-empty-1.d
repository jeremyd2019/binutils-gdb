#as: --gsframe
#warning: skipping SFrame FDE; \.cfi_escape DW\_CFA\_expression with SP reg 7
#objdump: --sframe=.sframe
#name: CFI_escape with register of significance to SFrame
#...
Contents of the SFrame section .sframe:

  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
#?    CFA fixed FP offset: \-?\d+
#?    CFA fixed RA offset: \-?\d+
    Num FDEs: 0
    Num FREs: 0

#pass
