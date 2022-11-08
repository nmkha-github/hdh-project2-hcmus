#include "syscall.h"

int main()
{
  char s[255];
  int maxLength;
  PrintString("Nhap do dai toi da cua xau (toi da 100): ");
  maxLength = ReadInt();
  PrintString("Nhap xau: ");
  ReadString(s, maxLength);
  PrintString("Xau da nhap: ");
  PrintString(s);
  Halt();
}