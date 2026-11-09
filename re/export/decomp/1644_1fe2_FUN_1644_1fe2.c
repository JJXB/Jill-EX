// FUN_1644_1fe2 @ 1644:1fe2

int __cdecl16far FUN_1644_1fe2(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined1 local_12 [16];
  
  if (param_2 == 0) {
    if (DAT_2cff_120e != 0) {
      FUN_1353_0b64(DAT_2cff_b7f2,DAT_2cff_b7f4,5,0xffff);
      FUN_2b85_0085(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),local_12);
      param_2 = FUN_1353_074e(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 4,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 4,1,
                              local_12);
    }
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 3) {
        FUN_1415_06eb();
        DAT_2cff_bcaa = 2;
        param_2 = 0;
      }
      else {
        param_2 = *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
        if (param_2 != DAT_2cff_83a6) {
          if (DAT_2cff_3f0e == 0) {
            iVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 0x32;
          }
          else {
            iVar1 = 5;
          }
          FUN_1581_0178(4,iVar1);
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) != 0) {
            DAT_2cff_83a6 = *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
          }
          if (DAT_2cff_96ac == '\x17') {
            FUN_26f4_1ad4(0,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
          }
          else {
            DAT_2cff_96ad = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
            DAT_2cff_96af = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + -0x10;
            DAT_2cff_96b9 = 4;
            DAT_2cff_96bd = 0;
          }
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f) != 0 ||
              *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f) != 0) {
            FUN_2b9f_0008((undefined1 *)&DAT_2cff_bb1e,0x2cff,
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f));
          }
          param_2 = 1;
        }
      }
    }
  }
  else if (param_2 == 2) {
    if ((((DAT_2cff_96ad <
           *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
           *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f)) &&
         (*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) <
          DAT_2cff_96ad + DAT_2cff_96b5)) &&
        (DAT_2cff_96af <
         *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
         *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f))) &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) < DAT_2cff_96af + DAT_2cff_96b7
       )) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_96ad;
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = DAT_2cff_96af;
    }
    param_2 = 0;
  }
  return param_2;
}


