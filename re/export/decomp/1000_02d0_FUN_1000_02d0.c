// FUN_1000_02d0 @ 1000:02d0

void __cdecl16far FUN_1000_02d0(void)

{
  char *pcVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int in_DX;
  int *piVar5;
  char *pcVar6;
  
  iVar2 = DAT_2cff_0077;
  pcVar6 = (char *)0x0;
  DAT_2cff_0071 = (int *)FUN_2ae9_000a(DAT_2cff_0079);
  DAT_2cff_0073 = in_DX;
  if (DAT_2cff_0071 == (int *)0x0 && in_DX == 0) {
    FUN_1000_01af();
    return;
  }
  iVar4 = -1;
  piVar3 = DAT_2cff_0071;
  do {
    piVar5 = piVar3;
    *piVar5 = (int)pcVar6;
    piVar5[1] = iVar2;
    do {
      if (iVar4 == 0) break;
      iVar4 = iVar4 + -1;
      pcVar1 = pcVar6;
      pcVar6 = pcVar6 + 1;
    } while (*pcVar1 != '\0');
    piVar3 = piVar5 + 2;
    if (*pcVar6 == '\0') {
      piVar5[2] = 0;
      piVar5[3] = 0;
      return;
    }
  } while( true );
}


