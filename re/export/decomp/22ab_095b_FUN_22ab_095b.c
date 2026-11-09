// FUN_22ab_095b @ 22ab:095b

void __cdecl16far FUN_22ab_095b(void)

{
  int iVar1;
  int iVar2;
  int in_DX;
  
  iVar1 = FUN_2b4b_000c(0x175c,0x2cff,0x8000);
  if (-1 < iVar1) {
    iVar2 = FUN_2c44_0001(iVar1);
    if ((0 < in_DX) || ((in_DX == 0 && (iVar2 != 0)))) {
      FUN_2b58_000d(iVar1,(undefined1 *)&DAT_2cff_b5ea,0x2cff,0x78);
      FUN_2b58_000d(iVar1,(undefined2 *)&DAT_2cff_b7ca,0x2cff,0x28);
      FUN_2b58_000d(iVar1,(undefined1 *)&DAT_2cff_bab4,0x2cff,0x48);
      iVar2 = FUN_2b58_000d(iVar1,(undefined2 *)&DAT_2cff_3dce,0x2cff,0x16);
      if (iVar2 < 0) {
        DAT_2cff_3dce = 1;
      }
      goto LAB_22ab_0a2e;
    }
  }
  for (iVar2 = 0; iVar2 < 10; iVar2 = iVar2 + 1) {
    ((undefined1 *)&DAT_2cff_b5ea)[iVar2 * 10] = 0;
    ((undefined2 *)&DAT_2cff_b7cc)[iVar2 * 2] = 0;
    ((undefined2 *)&DAT_2cff_b7ca)[iVar2 * 2] = 0;
  }
  for (iVar2 = 0; iVar2 < 6; iVar2 = iVar2 + 1) {
    ((undefined1 *)&DAT_2cff_bab4)[iVar2 * 0xc] = 0;
  }
  DAT_2cff_3dce = 1;
LAB_22ab_0a2e:
  FUN_2b53_0006(iVar1);
  return;
}


