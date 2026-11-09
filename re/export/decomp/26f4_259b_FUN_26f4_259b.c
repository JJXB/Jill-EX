// FUN_26f4_259b @ 26f4:259b

void __cdecl16far FUN_26f4_259b(void)

{
  int iVar1;
  bool bVar2;
  
  if ((DAT_2cff_b662 != '\0') &&
     (iVar1 = DAT_2cff_b6b4 + -1, bVar2 = DAT_2cff_b6b4 < 1, DAT_2cff_b6b4 = iVar1, bVar2)) {
    DAT_2cff_b662 = '\0';
    DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
  }
  return;
}


