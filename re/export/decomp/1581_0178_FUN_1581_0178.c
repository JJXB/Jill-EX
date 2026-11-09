// FUN_1581_0178 @ 1581:0178

void __cdecl16far FUN_1581_0178(int param_1,int param_2)

{
  if (DAT_2cff_3f0e == 0) {
    if ((((param_2 < 0x80) &&
         (((undefined2 *)&DAT_2cff_3fda)[param_2 * 2] != 0 ||
          ((undefined2 *)&DAT_2cff_3fdc)[param_2 * 2] != 0)) &&
        ((int)DAT_2cff_0be6 != 0 || DAT_2cff_0be6._2_2_ != 0)) &&
       ((int)DAT_2cff_0bea != 0 || DAT_2cff_0bea._2_2_ != 0)) {
      FUN_1530_00fe(param_1,((undefined2 *)&DAT_2cff_3fda)[param_2 * 2],
                    ((undefined2 *)&DAT_2cff_3fdc)[param_2 * 2]);
    }
  }
  else {
    if (*(char *)(param_2 + 0x1212) != '\0') {
      param_2 = (int)*(char *)(param_2 + 0x1212);
    }
    if (((DAT_2cff_0bf9 == 0) || (DAT_2cff_3e40 <= param_1)) &&
       ((((undefined2 *)&DAT_2cff_3f10)[param_2 * 2] != 0 ||
         ((undefined2 *)&DAT_2cff_3f12)[param_2 * 2] != 0 &&
        ((param_2 < 0x32 && (DAT_2cff_3e26 != 0)))))) {
      FUN_15ed_042f();
      FUN_15ed_038c(((undefined2 *)&DAT_2cff_3f10)[param_2 * 2],
                    ((undefined2 *)&DAT_2cff_3f12)[param_2 * 2],
                    ((undefined2 *)&DAT_2cff_3ea6)[param_2],((undefined2 *)&DAT_2cff_3e42)[param_2])
      ;
      DAT_2cff_3e40 = param_1;
    }
  }
  return;
}


