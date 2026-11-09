// FUN_2b67_0002 @ 2b67:0002

/* WARNING: Removing unreachable block (ram,0x0002b75c) */
/* WARNING: Removing unreachable block (ram,0x0002b7bf) */

int __cdecl16far FUN_2b67_0002(int param_1,char *param_2,int param_3)

{
  uint *puVar1;
  char *pcVar2;
  char cVar3;
  char *pcVar4;
  int iVar5;
  int iVar6;
  undefined2 unaff_SS;
  char *local_90;
  undefined2 uVar7;
  int local_8c;
  char *local_88;
  char local_84 [130];
  
  if (param_3 + 1U < 2) {
    param_3 = 0;
  }
  else if ((*(uint *)(param_1 * 2 + 0x2954) & 0x8000) == 0) {
    puVar1 = (uint *)(param_1 * 2 + 0x2954);
    *puVar1 = *puVar1 & 0xfdff;
    local_88 = param_2;
    local_8c = param_3;
    _local_90 = (char *)CONCAT22(unaff_SS,local_84);
    while( true ) {
      pcVar4 = local_88;
      if (local_8c == 0) break;
      local_8c = local_8c + -1;
      local_88 = (char *)CONCAT22(local_88._2_2_,(char *)local_88 + 1);
      cVar3 = *pcVar4;
      if (cVar3 == '\n') {
        *_local_90 = '\r';
        uVar7 = (undefined2)((ulong)_local_90 >> 0x10);
        _local_90 = (char *)CONCAT22(uVar7,local_90 + 1);
      }
      *_local_90 = cVar3;
      pcVar2 = local_90 + 1;
      uVar7 = (undefined2)((ulong)_local_90 >> 0x10);
      _local_90 = (char *)CONCAT22(uVar7,pcVar2);
      iVar5 = -(uint)(pcVar2 < local_84);
      if ((-1 < iVar5) && ((iVar5 != 0 || (0x7f < (uint)((int)pcVar2 - (int)local_84))))) {
        iVar5 = FUN_2b7d_0008(param_1,local_84);
        if (iVar5 != (int)pcVar2 - (int)local_84) {
          return ((param_3 - local_8c) + iVar5) - ((int)pcVar2 - (int)local_84);
        }
        _local_90 = (char *)CONCAT22(unaff_SS,local_84);
      }
    }
    iVar5 = (int)local_90 - (int)local_84;
    if ((iVar5 != 0) && (iVar6 = FUN_2b7d_0008(param_1,local_84), iVar6 != iVar5)) {
      param_3 = (param_3 + iVar6) - iVar5;
    }
  }
  else {
    param_3 = FUN_2b7d_0008(param_1,(char *)param_2,param_2._2_2_,param_3);
  }
  return param_3;
}


