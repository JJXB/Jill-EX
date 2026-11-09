// caseD_0 @ 1fae:0a9f

void switchD_2000:c53e::caseD_0(void)

{
  int *piVar1;
  int iVar2;
  undefined2 uVar3;
  int unaff_BP;
  int unaff_SI;
  undefined2 unaff_SS;
  
  FUN_22ab_05b4();
  DAT_2cff_b6b4 = 0x19;
  iVar2 = FUN_26f4_2227();
  if (iVar2 == 3) {
    *(undefined2 *)(unaff_BP + -0x2c) = 1;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 2;
  }
  iVar2 = FUN_26f4_2227();
  if (iVar2 == 3) {
    *(undefined2 *)(unaff_BP + -0x2c) = 1;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 0;
  }
  if (DAT_2cff_3c7a == 0) {
    if (DAT_2cff_3c78 != 0) {
      if ((((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + unaff_SI * 0x1f) & 0xf) == 0) &&
          (iVar2 = FUN_26f4_2227(), iVar2 != 0)) && (iVar2 = FUN_26f4_2227(), iVar2 == 0)) {
        *(undefined2 *)(unaff_BP + -0x2c) = 1;
        FUN_26f4_1ad4();
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 3;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 3;
      }
      if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) == 0) &&
         (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) == 0)) {
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) + DAT_2cff_3c78 * 2 < -3)
        {
          iVar2 = -3;
        }
        else {
          iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) + DAT_2cff_3c78 * 2;
        }
        if (iVar2 < 3) {
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) + DAT_2cff_3c78 * 2 < -3
             ) {
            iVar2 = -3;
          }
          else {
            iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) +
                    DAT_2cff_3c78 * 2;
          }
        }
        else {
          iVar2 = 3;
        }
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = iVar2;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) = 0;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) = 0;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + unaff_SI * 0x1f) = 3;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) < 2) {
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) < -1) {
            *(undefined2 *)(unaff_BP + -2) = 0xfffe;
          }
        }
        else {
          *(undefined2 *)(unaff_BP + -2) = 2;
        }
      }
    }
  }
  else {
    DAT_2cff_3d9e = 1;
    *(undefined2 *)(unaff_BP + -0x2c) = 1;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
    iVar2 = FUN_26f4_1a89();
    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = -(iVar2 * 4 + 0x10);
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 0;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) = DAT_2cff_3c76;
    FUN_1581_0178();
  }
  if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) != 0) {
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) =
         *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f);
  }
  if (((DAT_2cff_3c7e != 0) &&
      ((((undefined2 *)&DAT_2cff_be2c)
        [*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f)] & 1) != 0)) &&
     (DAT_2cff_3da0 = 1, *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) != 0)) {
    *(undefined2 *)(unaff_BP + -0x30) = 0;
    *(undefined2 *)(unaff_BP + -0x2a) = 0;
    while (*(int *)(unaff_BP + -0x2a) < DAT_2cff_becc) {
      *(int *)(unaff_BP + -0x30) =
           *(int *)(unaff_BP + -0x30) +
           (uint)(((undefined1 *)&DAT_2cff_96ac)
                  [((undefined2 *)&DAT_2cff_bcac)[*(int *)(unaff_BP + -0x2a)] * 0x1f] == '2');
      *(int *)(unaff_BP + -0x2a) = *(int *)(unaff_BP + -0x2a) + 1;
    }
    iVar2 = FUN_26f4_1a89();
    if (*(int *)(unaff_BP + -0x30) < iVar2) {
      FUN_26f4_1892();
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) * 6;
      FUN_1581_0178();
    }
    else {
      iVar2 = FUN_26f4_1a38();
      if (iVar2 == 0) {
        FUN_1581_0178();
      }
      else {
        uVar3 = FUN_26f4_1892();
        *(undefined2 *)(unaff_BP + -0x2a) = uVar3;
        if (*(int *)(unaff_BP + -0x2a) != 0) {
          iVar2 = FUN_26f4_2227(*(undefined2 *)(unaff_BP + -0x2a));
          if (iVar2 == 1) {
            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
                 *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) << 3;
            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + (DAT_2cff_bb1c + -1) * 0x1f) = 3;
            FUN_1581_0178();
          }
          else {
            FUN_26f4_1863();
            FUN_26f4_1a13();
            FUN_1581_0178();
          }
        }
      }
    }
  }
  piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bd + unaff_SI * 0x1f);
  *piVar1 = *piVar1 + 1;
  if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + unaff_SI * 0x1f) != 0) {
    *(undefined2 *)(unaff_BP + -0x2c) = 1;
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + unaff_SI * 0x1f);
    *piVar1 = *piVar1 + -1;
  }
  FUN_26f4_23e1();
  FUN_2cff_2afd();
  FUN_1fae_1d0f();
  return;
}


