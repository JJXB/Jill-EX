// FUN_2cc3_004d @ 2cc3:004d

void FUN_2cc3_004d(int *param_1,int *param_2)

{
  byte bVar1;
  char cVar3;
  int iVar2;
  
  iVar2 = *param_2;
  if (iVar2 != *param_1) {
    FUN_1000_0414();
    *param_1 = iVar2;
  }
  cVar3 = (char)((uint)iVar2 >> 8);
  bVar1 = (char)iVar2 + 1;
  iVar2 = CONCAT11(cVar3,bVar1);
  if (DAT_2cff_2a32 <= bVar1) {
    iVar2 = (uint)(byte)(cVar3 + 1) << 8;
  }
  *param_2 = iVar2;
  return;
}


