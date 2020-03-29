#include "xbox.h"
#include "xboxkrnl.h"
#include "BootString.h"
#include "BootParser.h"
#include "boot.h"

enum {
	VIDEO_MODE_UNKNOWN=-1,
	VIDEO_MODE_640x480=0,
	VIDEO_MODE_640x576,
	VIDEO_MODE_720x576,
	VIDEO_MODE_800x600,
	VIDEO_MODE_1024x576,
	VIDEO_MODE_COUNT
};

int sprintf(char * buf, const char *fmt, ...);

int ParseConfig(char *szPath,char *szBuffer, CONFIGENTRY *entry) {
	char *szLine;
	char *szTmp;
	BYTE VideoStandard[4];
	char *ptr;
	//,ptr1;
	int i;
	
        szLine = (char *)MmAllocateContiguousMemoryEx(MAX_LINE,MIN_KERNEL,
	                        MAX_KERNEL, 0, PAGE_READWRITE);
        szTmp = (char *)MmAllocateContiguousMemoryEx(MAX_LINE,MIN_KERNEL,
	                        MAX_KERNEL, 0, PAGE_READWRITE);

	for(i = 0; i < 4 ; i++) {
		VideoStandard[i] = I2CTransmitByteGetReturn(0x54, 0x58 + i);
	}
	
	memset(entry,0,sizeof(CONFIGENTRY));
	
	ptr = szBuffer;
	ptr = HelpGetToken(szBuffer,10);
	entry->nValid = 1;
	HelpCopyUntil(entry->szPath,szPath,MAX_LINE);
	while(1) {
		memcpy(szLine,ptr,HelpStrlen(ptr));
		if(HelpStrlen(ptr) < MAX_LINE) {
			if(HelpStrncmp(ptr,"Romfile",HelpStrlen("Romfile")) == 0)  {
				HelpGetParm(szTmp, ptr);
				HelpCopyUntil(entry->szRom,szPath,MAX_LINE);
				HelpCopyUntil(HelpScan0(entry->szRom),szTmp,MAX_LINE);
			}
			if(HelpStrncmp(ptr,"RC4Key",HelpStrlen("RC4Key")) == 0) {
				HelpGetParm(entry->szRC4Key, ptr);
			}
		} else {
			entry->nValid = 0;
		}
		ptr = HelpGetToken(0,10);
		if(*ptr == 0) break;
	}

	MmFreeContiguousMemory(szLine);
	MmFreeContiguousMemory(szTmp);

	return entry->nValid;
}

void PrintConfig(CONFIGENTRY *entry) {
        dprintf("path \"%s\"\n", entry->szPath);
        dprintf("kernel \"%s\"\n", entry->szRom);
        dprintf("initrd \"%s\"\n", entry->szRC4Key);
}

