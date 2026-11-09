// FUN_2ca9_000d @ 2ca9:000d

void FUN_2ca9_000d(int param_1,int param_2,undefined2 *param_3)

{
  undefined2 uVar1;
  
  uVar1 = CONCAT11(DAT_2cff_2a2e,0x20);
  for (; param_2 <= param_1; param_2 = param_2 + 1) {
    *param_3 = uVar1;
    param_3 = (undefined2 *)CONCAT22(param_3._2_2_,(undefined2 *)param_3 + 1);
  }
  return;
}


