#include "syscall.h"

int main() 
{ 
    int number;
    number = ReadInt();

    PrintInt(number);
    Halt(); 
}
