// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

// Input: - User space address (int) 
// - Limit of buffer (int) 
// Output:- Buffer (char*) 
// Purpose: Copy buffer from User memory space to System memory space 
char* User2System(int virtAddr, int limit)
{
    int i;// index 
    int oneChar;
    char* kernelBuf = NULL;
    kernelBuf = new char[limit + 1];//need for terminal string 
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    //printf("\n Filename u2s:"); 
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        //printf("%c",kernelBuf[i]); 
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Input: - User space address (int) 
// - Limit of buffer (int) 
// - Buffer (char[]) 
// Output:- Number of bytes copied (int) 
// Purpose: Copy buffer from System memory space to User memory space 
int System2User(int virtAddr, int len, char* buffer)
{
    if (len < 0) return -1;
    if (len == 0)return len;
    int i = 0;
    int oneChar = 0;
    do {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

//Di chuyen thanh ghi ve sau 4 byte de nap lenh ke tiep
//Cac buoc:
//B1: Ghi vao PrevPcReg <- gia tri PCReg hien tai
//B2: Ghi vao PCReg hien tai <- gia tri NextPCReg
//B3: Di chuyen thanh ghi ve sau 4 byte (ghi vao NextPCReg <- gia tri NextPCReg + 4)
void Increase_ProgramCounter()
{
	/*
	machine/mipssim.cc cuoi ham "void Machine::OneInstruction(Instruction *instr)"
	
	 // Advance program counters.
	    registers[PrevPCReg] = registers[PCReg];	// for debugging, in case we
							// are jumping into lala-land
	    registers[PCReg] = registers[NextPCReg];
	    registers[NextPCReg] = pcAfter;
	*/
    int pcAfter = machine->ReadRegister(NextPCReg) + 4;
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, pcAfter);
}

void Exception_ReadInt()
{

    const int maxlen = 11; // max len(int) = 11 do int: [-2147483648 , 2147483647] 
    char num_string[maxlen] = {0};
    long long ret = 0;

    for (int i = 0; i < maxlen; i++) {
        char c = 0; //biến lưu giá trị khi đọc từ console nhưng ở dạng char đọc từng ký tự
        ptrSynchConsole->Read(&c,1);
        if (c >= '0' && c <= '9'){
		num_string[i] = c;
	}
        else{
		if (i == 0 && c == '-'){
			num_string[i] = c;
		}
        	else break; //nếu nó khác số hoặc dấu - thì dừng đọc
	}
    }
    int i = (num_string[0] == '-') ? 1 : 0;
    while (i < maxlen && num_string[i] >= '0' && num_string[i] <= '9') //chuyển từng ký tự sang số
        ret = ret*10 + num_string[i++] - '0';
    ret = (num_string[0] == '-') ? (-ret) : ret;
    machine->WriteRegister(2, (int)ret);
}

void Exception_Halt()
{
    DEBUG('a', "\n Shutdown, initiated by user program.");
    printf("\n\n Shutdown, initiated by user program.");
    interrupt->Halt();
}

void Exception_PrintInt()
{
    int n = machine->ReadRegister(4); //đọc số đã được gán sẵn
    const int maxlen = 11; // giá trị độ dài tối đa kiểu int
    char num_string[maxlen] = {0};
    int tmp[maxlen] = {0}, i = 0, j = 0;
    if (n < 0) { //xử lý số âm
        n = -n; //nếu âm chuyển về dương để xử lý
        num_string[i++] = '-'; //chuỗi số xuất ra thêm dấu âm
    }
    do { //chuyển số sang mảng int để dễ thao tác
        tmp[j++] = n%10; 
        n /= 10;
    } while (n);
    while (j) // chuyển mảng int sang char
    	num_string[i++] = '0' + (char)tmp[--j];
    ptrSynchConsole->Write(num_string,i); //ghi ra màn hình char và độ dài mảng
    machine->WriteRegister(2, 0);
}


void Exception_ReadChar()
{
    char ch = 0;
    ptrSynchConsole->Read(&ch,1); // đọc 1 ký tự từ console
    machine->WriteRegister(2, (int)ch);
}

void Exception_PrintChar()
{
    char ch = (char)machine->ReadRegister(4); //lấy ký tự đã được gán sẵn
    ptrSynchConsole->Write(&ch,1); // ghi 1 ký tự ra màn hình console
    machine->WriteRegister(2, 0);
}

void Exception_ReadString()
{
	int virtAddr, length;
	char* buffer = new char[255];
	virtAddr = machine->ReadRegister(4); 
	length = machine->ReadRegister(5); 

	buffer = User2System(virtAddr, length); 

	ptrSynchConsole->Read(buffer, length); 

	System2User(virtAddr, length, buffer); 
	delete buffer; 
}

void Exception_PrintString()
{
	int virtAddr, length = 0;
	char* buffer;
	virtAddr = machine->ReadRegister(4); 
	buffer = User2System(virtAddr, 255); 

	//Tinh do dai chuoi
	while (buffer[length] != 0) length++;

	ptrSynchConsole->Write(buffer, length + 1); 
	delete buffer; 
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which) {
    case NoException:
        return;
    case SyscallException:
        switch (type) {
        case SC_Halt:
            Exception_Halt();
            break;
        //code mau cong hai so
        case SC_Sub:
            int op1, op2, result;
            op1 = machine->ReadRegister (4);
            op2 = machine->ReadRegister (5);
            result = op1 - op2;
            machine->WriteRegister (2, result);
            interrupt->Halt();
            break;
        case SC_ReadInt:
            Exception_ReadInt();
            Increase_ProgramCounter();
            break;
        case SC_PrintInt:
            Exception_PrintInt();
            Increase_ProgramCounter();
            break;
        case SC_ReadChar:
            Exception_ReadChar();
            Increase_ProgramCounter();
            break;
        case SC_PrintChar:
            Exception_PrintChar();
            Increase_ProgramCounter();
            break;
        case SC_ReadString:
            Exception_ReadString();
            Increase_ProgramCounter();
            break;
        case SC_PrintString:
            Exception_PrintString();
            Increase_ProgramCounter();
            break;
        default:
            printf("\n Unexpected user mode exception (%d %d)", which,
                type);
            interrupt->Halt();
        }
        break;

    case PageFaultException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case BusErrorException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case AddressErrorException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case OverflowException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        interrupt->Halt();
        break;
    default:
        DEBUG('a', "\nERROR. Can not run this program.......");
        printf("\nError. Can not run this program....... ");
        break;
    }
}