/*
   Xbox XBE bootloader

    Original code by Michael Steil & anonymous
    VESA Framebuffer code by Milosch Meriac
	Modified to boot shadow bioses by Phoenix

	Note that this is a very modified version, not created
	nor endorsed by the xbox-linux team.
*/

#include "printf.c"
#include "consts.h"
#include "xboxkrnl.h"
#include "xbox.h"
#include "boot.h"
#include "BootString.h"
#include "BootParser.h"
#include "BootVideo.h"
#include "config.h"

#define LOCK_FILE_EXISTS 1
#define LOCK_FILE_CREATED 0

extern void DoIntro();

volatile CURRENT_VIDEO_MODE_DETAILS currentvideomodedetails;

/* Loads the kernel image file into contiguous physical memory */
static NTSTATUS LoadShadowROM(PVOID Filename, long *FilePos, long *FileSize) 
{
	HANDLE SourceFile;
	PVOID VirtFile = 0;

	// Temporary buffer for use with ReadFile
	PVOID ReadBuffer = 0;

	// Error code from an NT kernel call
	NTSTATUS Error = 1;

	// ANSI_STRING of the kernel image filename
	ANSI_STRING FileString;

	// Object attributes of the kernel image file
	OBJECT_ATTRIBUTES FileAttributes;

	// IO status block (for reading the file)
	IO_STATUS_BLOCK IoStatusBlock;

	// Size of the kernel file
	ULONGLONG TempSize;

	// Read pointer to the kernel file
	PUCHAR ReadPtr;

	ULONG ReadSize;

	int i;
	
	ReadBuffer = MmAllocateContiguousMemoryEx(READ_CHUNK_SIZE,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	if (!ReadBuffer) goto RealError;

	// Make an ANSI_STRING out of the kernel image filename 
	RtlInitAnsiString(&FileString, Filename);

	// Kernel object attributes (ignore case, use system root) 
	FileAttributes.Attributes = OBJ_CASE_INSENSITIVE;
	FileAttributes.ObjectName = &FileString;
	FileAttributes.RootDirectory = NULL;

	// Open a file handle to the kernel image 
	Error = NtCreateFile(&SourceFile, 0x80100080 /* GENERIC_READ |
		SYNCHRONIZE | FILE_READ_ATTRIBUTES */, &FileAttributes,
		&IoStatusBlock, NULL, 0, 7 /* FILE_SHARE_READ | FILE_SHARE_WRITE |
		FILE_SHARE_DELETE*/, 1 /* FILE_OPEN */, 0x60 /* FILE_NON_DIRECTORY_FILE |
		FILE_SYNCHRONOUS_IO_NONALERT */);

	if (!NT_SUCCESS(Error)) goto RealError;

	TempSize = *FileSize;

	VirtFile = MmAllocateContiguousMemoryEx((ULONG) SHADOW_ROM_SIZE,MIN_SHADOW_ROM, MAX_SHADOW_ROM, 0, PAGE_READWRITE);

	if (!VirtFile) goto RealError;

	ReadPtr = (PUCHAR) VirtFile;
	
	for (i=0; i < TempSize; i+=READ_CHUNK_SIZE, ReadPtr+=READ_CHUNK_SIZE) 
	{
		ReadSize = READ_CHUNK_SIZE;
		Error = NtReadFile(SourceFile, NULL, NULL, NULL, &IoStatusBlock, ReadBuffer, ReadSize, NULL);
		memcpy(ReadPtr, ReadBuffer, ReadSize);
		if (!NT_SUCCESS(Error)) goto RealError;
	}

	*FileSize = i; // just an estimation
	*FilePos = (int)VirtFile;

	MmFreeContiguousMemory(ReadBuffer);

	Error = STATUS_SUCCESS;

RealError:
	return Error;
}

NTSTATUS CheckOrCreateLock() 
{
	HANDLE SourceFile;

	// Error code from an NT kernel call
	NTSTATUS Error = LOCK_FILE_CREATED;

	// ANSI_STRING of the kernel image filename
	ANSI_STRING FileString;

	// Object attributes of the kernel image file
	OBJECT_ATTRIBUTES FileAttributes;

	// IO status block (for reading the file)
	IO_STATUS_BLOCK IoStatusBlock;
	
	char *path;
	char *filename;

    path = (char *)MmAllocateContiguousMemoryEx(BUFFERSIZE,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
    filename = (char *)MmAllocateContiguousMemoryEx(BUFFERSIZE,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	
	memset(path,0,BUFFERSIZE);
	/* get the directory of the bootloader executable */
	HelpCopyUntil(path, XeImageFileName->Buffer, XeImageFileName->Length);
	HelpStrrchr(path, '\\')[1] = 0;
	/* read the config file from there */
	HelpCopyUntil(filename, path, BUFFERSIZE);
	HelpCopyUntil(HelpScan0(filename), "introflag.lock", BUFFERSIZE);


	// Make an ANSI_STRING out of the kernel image filename 
	RtlInitAnsiString(&FileString, filename);

	// Kernel object attributes (ignore case, use system root) 
	FileAttributes.Attributes = OBJ_CASE_INSENSITIVE;
	FileAttributes.ObjectName = &FileString;
	FileAttributes.RootDirectory = NULL;

	// Open a file handle to the kernel image 
	Error = NtCreateFile(&SourceFile, 0x40100116 /* GENERIC_WRITE |
		SYNCHRONIZE | FILE_READ_ATTRIBUTES */, &FileAttributes,
		&IoStatusBlock, NULL, 0, 7 /* FILE_SHARE_READ | FILE_SHARE_WRITE |
		FILE_SHARE_DELETE*/, 2 /* FILE_CREATE */, 0x60 /* FILE_NON_DIRECTORY_FILE |
		FILE_SYNCHRONOUS_IO_NONALERT */);

	if (!NT_SUCCESS(Error)) 
	{
		Error = LOCK_FILE_EXISTS;
	}
	MmFreeContiguousMemory(path);	
	MmFreeContiguousMemory(filename);
	return Error;
}

void DoSplash( PVOID Framebuffer )
{
	PUCHAR Image;
	PUCHAR FileImage;
	PUCHAR TempPtr;

	HANDLE SourceFile;

	// Error code from an NT kernel call
	NTSTATUS Error = LOCK_FILE_CREATED;

	// ANSI_STRING of the kernel image filename
	ANSI_STRING FileString;

	// Object attributes of the kernel image file
	OBJECT_ATTRIBUTES FileAttributes;

	// IO status block (for reading the file)
	IO_STATUS_BLOCK IoStatusBlock;

	char *path;
	char *filename;

	int i;
	int j;

	Image = (PVOID)MmAllocateContiguousMemoryEx(640*480*4, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	FileImage = (PVOID)MmAllocateContiguousMemoryEx(640*480*3, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);

    path = (char *)MmAllocateContiguousMemoryEx(BUFFERSIZE,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
    filename = (char *)MmAllocateContiguousMemoryEx(BUFFERSIZE,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	
	memset(path,0,BUFFERSIZE);
	/* get the directory of the bootloader executable */
	HelpCopyUntil(path, XeImageFileName->Buffer, XeImageFileName->Length);
	HelpStrrchr(path, '\\')[1] = 0;
	/* read the config file from there */
	HelpCopyUntil(filename, path, BUFFERSIZE);
	HelpCopyUntil(HelpScan0(filename), "phoenix.raw", BUFFERSIZE);

// Make an ANSI_STRING out of the kernel image filename 
	RtlInitAnsiString(&FileString, filename);

	// Kernel object attributes (ignore case, use system root) 
	FileAttributes.Attributes = OBJ_CASE_INSENSITIVE;
	FileAttributes.ObjectName = &FileString;
	FileAttributes.RootDirectory = NULL;

	// Open a file handle to the kernel image 
	Error = NtCreateFile(&SourceFile, 0x80100080 /* GENERIC_READ |
		SYNCHRONIZE | FILE_READ_ATTRIBUTES */, &FileAttributes,
		&IoStatusBlock, NULL, 0, 7 /* FILE_SHARE_READ | FILE_SHARE_WRITE |
		FILE_SHARE_DELETE*/, 1 /* FILE_OPEN */, 0x60 /* FILE_NON_DIRECTORY_FILE |
		FILE_SYNCHRONOUS_IO_NONALERT */);

	if (NT_SUCCESS(Error)) 
	{
		TempPtr = Image;
		Error = NtReadFile(SourceFile, NULL, NULL, NULL, &IoStatusBlock, FileImage, 640*480*24, NULL);
		for (i=0;i<640*480*3;i+=3)
		{
			*TempPtr = *(FileImage+2);
			*(TempPtr+1) = *(FileImage+1);
			*(TempPtr+2) = *(FileImage);
			TempPtr += 4;
			FileImage += 3;
		}
		memcpy(Framebuffer,Image,640*480*4);
	}	
	MmFreeContiguousMemory(Image);
	MmFreeContiguousMemory(FileImage);
	MmFreeContiguousMemory(path);	
	MmFreeContiguousMemory(filename);
	// Delay ~3secs
	for(i=0;i<40000;i++) for(j=0;j<40000;j++) ;
}

// Useful for debugging
void die() {
	while(1);
}

PHYSICAL_ADDRESS PhysicalRomPos;
PVOID EntryPoint2BL;

NTSTATUS GetConfig(CONFIGENTRY *entry);

void boot() {

	long ShadowRomPos;
	long RomSize;
	int i;
	int sum = 0;

	ULONG val;
	NTSTATUS Error;
	PUCHAR KernelParamPtr = 0;
	PVOID CopyPtr = 0;
	PVOID RC4State = 0;
	PVOID TempPtr = 0;
	PVOID Virt2BL = 0;
	PCHAR ParsePtr = 0;

	CONFIGENTRY entry;

	UCHAR RC4Key[16];

	currentvideomodedetails.m_nVideoModeIndex=VIDEO_MODE_640x480;
    currentvideomodedetails.m_pbBaseAddressVideo=(BYTE *)0xfd000000;
	currentvideomodedetails.m_dwFrameBufferStart = FRAMEBUFFER_START;

    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&currentvideomodedetails);
	
	framebuffer = (unsigned int*)(0xF0000000+*(unsigned int*)0xFD600800);
	memset(framebuffer,0,640*480*4);

	memset(&entry,0,sizeof(CONFIGENTRY));
	cx = 0;
	cy = 0;

	// parse the configuration file                         
	Error = GetConfig(&entry);

	if (!NT_SUCCESS(Error)) 
	{
		dprintf("Error reading config!\n");
		die();
	}

	// Only do intro the 1st time
	if (CheckOrCreateLock() == LOCK_FILE_CREATED)
	{
		DoIntro(framebuffer);
	}
	else
	{
		DoSplash(framebuffer);
	}

	// Load the kernel image into RAM 
	RomSize = MAX_KERNEL_SIZE;

	Error = LoadShadowROM(entry.szRom, &ShadowRomPos, &RomSize);

	if (!NT_SUCCESS(Error)) {
		dprintf("Error loading ROM\n");
		die();
	}

	// Get physical address of rom
	PhysicalRomPos = MmGetPhysicalAddress((PVOID)ShadowRomPos);

	// Allocate memory for the 2bl.  Has to be at 0x00400000
	Virt2BL = MmAllocateContiguousMemoryEx((ULONG) THE_2BL_SIZE,MIN_2BL, MAX_2BL, 0, PAGE_READWRITE);
	if (!Virt2BL) {
		dprintf("\nNo memory for 2BL!\n");
		die();
	}

	// Parse the RC4Key from the config file
	memset(RC4Key,0,16);
	ParsePtr = (PCHAR)(entry.szRC4Key);
 	for (i = 0; *ParsePtr && i < 16; i++) 
	{
		while(isspace(*ParsePtr)) 
		{
			++ParsePtr;
		}
		val = strtoul(ParsePtr, &ParsePtr, 0);
		sum += val;
		RC4Key[i] = (UCHAR)val;
	}

	// If the RC4 key wasn't blank
	if (sum > 0)
	{
		// Allocate memory for the RC4 state array
		RC4State = MmAllocateContiguousMemoryEx(0x100,MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
		if (!RC4State) {
			dprintf("\nNo memory for RC4 state!\n");
			die();
		}

		// Decrypt the 2bl
		XcRC4Key(RC4State, 0x10, RC4Key);
		CopyPtr = (PVOID)(ShadowRomPos + (SHADOW_ROM_SIZE - 0x6200));
		XcRC4Crypt(RC4State, 0x6000, CopyPtr);

		MmFreeContiguousMemory(RC4State);	
	}

	// Copy the 2bl to the appropriate location
	memcpy(Virt2BL, CopyPtr, THE_2BL_SIZE);

    // Patch in the EEprom key
	memcpy((PVOID)((ShadowRomPos + (SHADOW_ROM_SIZE - 0x6200)) + 0x64),XboxEEPROMKey, 16);
	memcpy((PVOID)(Virt2BL+0x64),XboxEEPROMKey, 16);
	
	// set the kernel param string
	KernelParamPtr = (PUCHAR)0x80400004;
	memcpy(KernelParamPtr, (PUCHAR)" /SHADOW /HDBOOT", 16);

	// Calculate the 2bl entry point
	TempPtr = (PVOID)(*(PULONG)Virt2BL + 0x8036FFFC);
	EntryPoint2BL = (PVOID)(*(PULONG)TempPtr) + 0x80000000;

	// Clear the framebuffer
	memset(framebuffer,0,640*480*4);

	// Call the 2bl
	__asm(
		"mov	PhysicalRomPos, %ecx\n"
		"mov	EntryPoint2BL, %eax\n"
        "call   %eax\n" 
	);

}

