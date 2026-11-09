// FUN_2ce5_0008 @ 2ce5:0008

char * __cdecl16far FUN_2ce5_0008(char *param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  undefined2 uVar4;
  
  uVar4 = (undefined2)((ulong)param_1 >> 0x10);
  pcVar2 = (char *)param_1;
  while( true ) {
    pcVar3 = pcVar2;
    cVar1 = *pcVar3;
    if (cVar1 == '\0') break;
    pcVar2 = pcVar3 + 1;
    if ((byte)(cVar1 + 0x9fU) < 0x1a) {
      *pcVar3 = cVar1 + -0x20;
    }
  }
  return (char *)param_1;
}


