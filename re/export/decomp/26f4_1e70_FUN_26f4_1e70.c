// FUN_26f4_1e70 @ 26f4:1e70

undefined2 __cdecl16far FUN_26f4_1e70(int param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  undefined2 uVar2;
  
  iVar1 = FUN_26f4_2227(param_1,param_2,param_3,3);
  if (iVar1 == 3) {
    FUN_26f4_1ad4(param_1,param_2,param_3);
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76;
    uVar2 = 1;
  }
  else {
    iVar1 = FUN_26f4_2227(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),param_3,3);
    if (iVar1 == 3) {
      FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    param_3);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0;
      uVar2 = 1;
    }
    else {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0;
      uVar2 = 0;
    }
  }
  return uVar2;
}


