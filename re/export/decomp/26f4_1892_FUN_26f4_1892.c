// FUN_26f4_1892 @ 26f4:1892

int __cdecl16far FUN_26f4_1892(int param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  
  ((undefined1 *)&DAT_2cff_96ac)[DAT_2cff_bb1c * 0x1f] = (char)param_1;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + DAT_2cff_bb1c * 0x1f) = param_2;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + DAT_2cff_bb1c * 0x1f) = param_3;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b5 + DAT_2cff_bb1c * 0x1f) =
       ((undefined2 *)&DAT_2cff_b6b6)[param_1];
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b7 + DAT_2cff_bb1c * 0x1f) =
       ((undefined2 *)&DAT_2cff_b740)[param_1];
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)(DAT_2cff_bb1c * 0x1f + -0x693f) = 0;
  iVar1 = DAT_2cff_bb1c * 0x1f;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar1) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar1) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c9 + DAT_2cff_bb1c * 0x1f) = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + DAT_2cff_bb1c * 0x1f) = 0;
  if (DAT_2cff_bb1c < 0x100) {
    DAT_2cff_bb1c = DAT_2cff_bb1c + 1;
  }
  return DAT_2cff_bb1c + -1;
}


