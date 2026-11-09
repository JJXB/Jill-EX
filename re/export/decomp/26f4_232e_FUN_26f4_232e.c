// FUN_26f4_232e @ 26f4:232e

uint __cdecl16far FUN_26f4_232e(int param_1,int param_2,int param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  uint local_c;
  
  local_c = 0xffff;
  for (iVar1 = param_3 >> 4;
      iVar1 < *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) + param_3 + 0xf >> 4;
      iVar1 = iVar1 + 1) {
    for (iVar2 = param_2 >> 4;
        iVar2 < *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + param_2 + 0xf >> 4;
        iVar2 = iVar2 + 1) {
      local_c = local_c & *(uint *)(((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar1] * 8 +
                                   -0x7c12) & param_4;
    }
  }
  return local_c;
}


