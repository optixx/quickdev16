rm *.obj
rm *.smc
wla-65816 -ov main.asm main.obj
wla-65816 -ov spx_snes.asm spx_snes.obj
wlalink -rvS main.link xmsnes.smc
