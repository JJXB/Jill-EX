// FUN_22ab_140b @ 22ab:140b

void __cdecl16far FUN_22ab_140b(int param_1,int param_2)

{
  if ((((-1 < param_1) && (param_1 < 0x80)) && (-1 < param_2)) && (param_2 < 0x40)) {
    if ((*(uint *)(((undefined2 *)&DAT_2cff_43a6)[param_1 * 0x40 + param_2] * 8 + -0x7c12) & 0x10)
        == 0) {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    *(undefined2 *)
                     (((undefined2 *)&DAT_2cff_43a6)[param_1 * 0x40 + param_2] * 8 + -0x7c14),
                    param_1 << 4,param_2 << 4);
    }
    else {
      FUN_1f28_0006(param_1,param_2,0);
    }
  }
  return;
}


