// FUN_1530_03a0 @ 1530:03a0

uint __cdecl16far FUN_1530_03a0(int param_1,char param_2,uint param_3)

{
  out(0x43,(char)param_1 * '@' + param_2 * '\x02' + '0');
  out(param_1 + 0x40,(undefined1)param_3);
  out(param_1 + 0x40,(char)(param_3 >> 8));
  return param_3 >> 8;
}


