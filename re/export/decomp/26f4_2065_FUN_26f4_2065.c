// FUN_26f4_2065 @ 26f4:2065

void __cdecl16far
FUN_26f4_2065(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5)

{
  uint uVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  
  uVar1 = FUN_26f4_1892(param_3,param_4,param_5);
  if (uVar1 != 0) {
    uVar3 = (undefined2)((ulong)uVar1 * 0x1f >> 0x10);
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + (int)((ulong)uVar1 * 0x1f)) = 0x40;
    uVar2 = FUN_2ce0_000e(param_1,param_2);
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + uVar1 * 0x1f) = uVar3;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + uVar1 * 0x1f) = uVar2;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + uVar1 * 0x1f) = 2;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + uVar1 * 0x1f) = 0xffff;
    FUN_26f4_020c(uVar1);
  }
  return;
}


