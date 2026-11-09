// FUN_2cc3_000d @ 2cc3:000d

undefined2 FUN_2cc3_000d(uint param_1)

{
  char cVar1;
  
  cVar1 = (char)((param_1 >> 1) / (uint)DAT_2cff_2a32);
  return CONCAT11(cVar1,(char)(param_1 >> 1) - cVar1 * DAT_2cff_2a32);
}


