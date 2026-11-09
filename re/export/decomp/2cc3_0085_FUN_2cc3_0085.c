// FUN_2cc3_0085 @ 2cc3:0085

void FUN_2cc3_0085(int param_1,undefined2 *param_2,undefined2 *param_3)

{
  undefined2 uVar1;
  undefined2 unaff_SS;
  undefined2 local_c;
  undefined2 local_a;
  undefined2 local_8;
  uint local_6;
  uint local_4;
  
  local_8 = FUN_2cf4_000f();
  local_6 = (uint)(param_3._2_2_ == DAT_2cff_2a37);
  if (local_6 != 0) {
    local_c = FUN_2cc3_000d((undefined2 *)param_3,param_3._2_2_);
  }
  local_4 = (uint)(param_2._2_2_ == DAT_2cff_2a37);
  if (local_4 != 0) {
    local_a = FUN_2cc3_000d((undefined2 *)param_2,param_2._2_2_);
  }
  while (param_1 != 0) {
    if (local_4 == 0) {
      uVar1 = *param_2;
      param_2 = (undefined2 *)CONCAT22(param_2._2_2_,(undefined2 *)param_2 + 1);
    }
    else {
      FUN_2cc3_004d(&local_8,unaff_SS,&local_a,unaff_SS);
      uVar1 = FUN_1000_0414();
    }
    if (local_6 == 0) {
      *param_3 = uVar1;
      param_3 = (undefined2 *)CONCAT22(param_3._2_2_,(undefined2 *)param_3 + 1);
      param_1 = param_1 + -1;
    }
    else {
      FUN_2cc3_004d(&local_8,unaff_SS,&local_c,unaff_SS);
      FUN_1000_0414();
      param_1 = param_1 + -1;
    }
  }
  FUN_1000_0414();
  return;
}


