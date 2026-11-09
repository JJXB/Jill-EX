// FUN_2b58_000d @ 2b58:000d

int __cdecl16far FUN_2b58_000d(int param_1,char *param_2,int param_3)

{
  uint *puVar1;
  char *pcVar2;
  char cVar3;
  char *pcVar4;
  int iVar5;
  char *pcVar6;
  char *pcVar7;
  char *pcVar8;
  undefined2 uVar9;
  char local_3;
  
  if ((param_3 + 1U < 2) || ((*(uint *)(param_1 * 2 + 0x2954) & 0x200) != 0)) {
    iVar5 = 0;
  }
  else {
    do {
      iVar5 = FUN_2b65_0002(param_1,(char *)param_2,param_2._2_2_,param_3);
      if ((iVar5 + 1U < 2) || ((*(uint *)(param_1 * 2 + 0x2954) & 0x8000) != 0)) {
        return iVar5;
      }
      pcVar7 = (char *)param_2;
      pcVar4 = (char *)param_2;
      do {
        while( true ) {
          pcVar8 = pcVar4;
          pcVar2 = pcVar7;
          pcVar7 = pcVar7 + 1;
          cVar3 = *pcVar2;
          pcVar6 = (char *)param_2;
          if (cVar3 == '\x1a') {
            FUN_2b82_0004(param_1,-iVar5,-(uint)(iVar5 != 0),2);
            puVar1 = (uint *)(param_1 * 2 + 0x2954);
            *puVar1 = *puVar1 | 0x200;
            goto LAB_2b58_00c6;
          }
          if (cVar3 == '\r') break;
          *pcVar8 = cVar3;
          iVar5 = iVar5 + -1;
          pcVar4 = pcVar8 + 1;
          if (iVar5 == 0) goto LAB_2b58_009c;
        }
        iVar5 = iVar5 + -1;
        pcVar4 = pcVar8;
      } while (iVar5 != 0);
      uVar9 = param_2._2_2_;
      FUN_2b65_0002(param_1,&local_3);
      *pcVar8 = local_3;
LAB_2b58_009c:
      pcVar8 = pcVar8 + 1;
    } while (pcVar8 == pcVar6);
LAB_2b58_00c6:
    iVar5 = (int)pcVar8 - (int)pcVar6;
  }
  return iVar5;
}


