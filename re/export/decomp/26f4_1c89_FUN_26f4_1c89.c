// FUN_26f4_1c89 @ 26f4:1c89

undefined2 __cdecl16far FUN_26f4_1c89(int param_1,undefined2 param_2,int param_3)

{
  int iVar1;
  undefined2 uVar2;
  int iVar3;
  
  iVar3 = 1;
  if (*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) < param_3) {
    iVar3 = 3;
  }
  iVar1 = FUN_26f4_2227(param_1,param_2,param_3,iVar3);
  if (iVar1 == iVar3) {
    FUN_26f4_1ad4(param_1,param_2,param_3);
    uVar2 = 1;
  }
  else {
    iVar1 = FUN_26f4_2227(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),param_3,
                          iVar3);
    if (iVar1 == iVar3) {
      FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    param_3);
      uVar2 = 2;
    }
    else {
      iVar1 = FUN_26f4_2227(param_1,param_2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),
                            iVar3);
      if (iVar1 == iVar3) {
        FUN_26f4_1ad4(param_1,param_2,
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
        uVar2 = 4;
      }
      else {
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}


