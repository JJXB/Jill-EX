// FUN_1069_0b5a @ 1069:0b5a

void __cdecl16far FUN_1069_0b5a(void)

{
  int iVar1;
  
  for (iVar1 = 0; iVar1 < 0x40; iVar1 = iVar1 + 1) {
    if (((undefined2 *)&DAT_2cff_312e)[iVar1 * 2] != 0 ||
        ((undefined2 *)&DAT_2cff_3130)[iVar1 * 2] != 0) {
      FUN_2c09_0008(((undefined2 *)&DAT_2cff_312e)[iVar1 * 2],
                    ((undefined2 *)&DAT_2cff_3130)[iVar1 * 2]);
      ((undefined2 *)&DAT_2cff_3130)[iVar1 * 2] = 0;
      ((undefined2 *)&DAT_2cff_312e)[iVar1 * 2] = 0;
    }
  }
  return;
}


