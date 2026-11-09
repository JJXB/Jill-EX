// FUN_1fae_298b @ 1fae:298b

int __cdecl16far FUN_1fae_298b(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined2 unaff_SS;
  uint local_10;
  int local_e [6];
  
  FUN_1000_05d8(0x11fb,0x2cff,local_e,unaff_SS);
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0) {
      iVar4 = 4;
    }
    else {
      iVar4 = 0;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb7a * 0x100 +
                            local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                            iVar4,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) != 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if (5 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
    }
    if (((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) & 7) == 0) &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) != 0)) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76 << 3;
    }
    else {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76 << 2;
    }
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if ((DAT_2cff_3c7a != 0) || (DAT_2cff_3c7e != 0)) {
      DAT_2cff_3d9e = 1;
      FUN_1581_0178(1,0xf);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfffa;
    }
    if (DAT_2cff_3c7e != 0) {
      DAT_2cff_3da0 = 1;
      FUN_1581_0178(1,0x22);
      FUN_26f4_1892(0x3e,((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f)) -
                         (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0)) *
                         0x18 + *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
           ((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f)) -
           (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0)) * 8;
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + (DAT_2cff_bb1c + -1) * 0x1f) = DAT_2cff_3c78 << 1
      ;
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 9) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -8) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff8;
      }
    }
    else {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 8;
    }
    iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    iVar2 = FUN_26f4_1d73(param_1,iVar4,
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (iVar2 == 0) {
      iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) != 0) {
      uVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
              *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      iVar2 = FUN_26f4_1d73(param_1,iVar4,uVar3);
      if (iVar2 == 0) {
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
          local_10 = uVar3 + 0x10 & 0xfff0;
        }
        else if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
          local_10 = ((uVar3 & 0xfff0) + 0x10) -
                     *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
        }
        if ((local_10 == uVar3) || (iVar4 = FUN_26f4_1d73(param_1,iVar4,local_10), iVar4 == 0)) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
        }
      }
    }
    iVar4 = FUN_26f4_232e(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),0x4000
                         );
    if (iVar4 != 0x4000) {
      FUN_26f4_05e7(0x100,1);
    }
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,0);
    param_2 = 1;
  }
  return param_2;
}


