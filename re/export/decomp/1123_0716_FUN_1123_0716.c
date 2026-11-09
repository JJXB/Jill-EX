// FUN_1123_0716 @ 1123:0716

void __cdecl16far FUN_1123_0716(void)

{
  byte bVar1;
  undefined2 uVar2;
  
  if ((DAT_2cff_3a4d & 0xfe) != 0) {
    DAT_2cff_3a4e = (uint)(DAT_2cff_3a4e == 0);
    DAT_2cff_3a50 = (uint)(DAT_2cff_3a50 == 0);
    FUN_1123_06ac();
    uVar2 = FUN_1123_0704();
    do {
      bVar1 = in(0x3da);
    } while ((bVar1 & 8) != 0);
    FUN_2ca4_000c(uVar2,(DAT_2cff_3a4b & 0xff00) + 0xc);
    FUN_2ca4_000c(uVar2,DAT_2cff_3a4b * 0x100 + 0xd);
    do {
      bVar1 = in(0x3da);
    } while ((bVar1 & 8) == 0);
  }
  return;
}


