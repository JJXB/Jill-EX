// FUN_26f4_2227 @ 26f4:2227

uint __cdecl16far FUN_26f4_2227(int param_1,int param_2,int param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  uint local_10;
  uint local_e;
  
  local_10 = 2;
  local_e = 0xffff;
  for (iVar1 = param_3 >> 4;
      iVar1 < *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) + param_3 + 0xf >> 4;
      iVar1 = iVar1 + 1) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
        ((undefined2 *)&DAT_2cff_b740)[(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f]] + 0xf
        >> 4 <= iVar1) {
      local_10 = 0;
    }
    for (iVar2 = param_2 >> 4;
        iVar2 < *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + param_2 + 0xf >> 4;
        iVar2 = iVar2 + 1) {
      local_e = local_e & (*(uint *)(((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar1] * 8 +
                                    -0x7c12) | local_10) & param_4;
    }
  }
  return local_e;
}


