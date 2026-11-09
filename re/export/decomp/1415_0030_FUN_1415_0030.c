// FUN_1415_0030 @ 1415:0030

void __cdecl16far FUN_1415_0030(int *param_1,int *param_2)

{
  byte bVar1;
  int iVar2;
  
  *param_1 = 0;
  *param_2 = 0;
  iVar2 = 0;
  out(0x201,0);
  do {
    bVar1 = in(0x201);
    *param_1 = *param_1 + (bVar1 & 1);
    bVar1 = in(0x201);
    *param_2 = *param_2 + (bVar1 & 2);
    iVar2 = iVar2 + 1;
    bVar1 = in(0x201);
  } while ((bVar1 & 3) != 0 && -1 < iVar2);
  *param_2 = *param_2 / 2;
  if (iVar2 < 0) {
    *param_1 = -1;
    *param_2 = -1;
  }
  return;
}


