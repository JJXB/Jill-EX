// FUN_26f4_2772 @ 26f4:2772

int __cdecl16far FUN_26f4_2772(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) -
          *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_2 * 0x1f);
  uVar3 = (int)uVar1 >> 0xf;
  uVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) -
          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_2 * 0x1f);
  uVar4 = (int)uVar2 >> 0xf;
  return ((uVar1 ^ uVar3) - uVar3) + ((uVar2 ^ uVar4) - uVar4);
}


