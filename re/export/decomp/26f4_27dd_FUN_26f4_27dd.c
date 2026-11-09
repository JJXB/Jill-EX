// FUN_26f4_27dd @ 26f4:27dd

int __cdecl16far FUN_26f4_27dd(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  int local_4;
  
  local_4 = 0;
  for (iVar2 = param_2 / 0x10;
      iVar2 <= (*(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + param_2) / 0x10;
      iVar2 = iVar2 + 1) {
    for (iVar3 = param_3 / 0x10;
        iVar3 <= (*(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) + param_3) / 0x10;
        iVar3 = iVar3 + 1) {
      if ((((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] & 0x3fff) == 0xa7) {
        ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] = 0xc000;
        iVar1 = local_4 + 1;
        bVar4 = local_4 == 0;
        local_4 = iVar1;
        if (bVar4) {
          FUN_1a41_09a8(iVar2 << 4,iVar3 << 4,5);
          FUN_1581_0178(2,0x31);
        }
      }
    }
  }
  return local_4;
}


