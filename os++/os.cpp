// Zenith26 - UEFI Shell Core

// --- Types ---
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef unsigned long long UINT64;
typedef long long          INT64;
typedef UINT64             UINTN;
typedef INT64              INTN;
typedef char16_t             CHAR16;
typedef unsigned char      BOOLEAN;
typedef void               VOID;

// --- EFI handles & status ---
typedef UINT64 EFI_STATUS;
typedef void*  EFI_HANDLE;

// --- Calling convention ---
#define EFIAPI __attribute__((ms_abi))

// --- Success codes ---
#define EFI_SUCCESS 0
#define EFI_DEVICE_ERROR 0x8000000000000007ULL

//null == void
#define NULL ((void*)0)

// --- Forward declarations ---
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct EFI_RUNTIME_SERVICES;
struct EFI_BOOT_SERVICES;
struct EFI_CONFIGURATION_TABLE;

// --- Colors ---
const UINTN GREEN = 0x02;
const UINTN WHITE = 0x0F;
const UINTN RED = 0x04;

// --- EFI_TABLE_HEADER ---
struct EFI_TABLE_HEADER {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
};
struct EFI_TIME {
    unsigned short Year;
    unsigned char  Month;
    unsigned char  Day;
    unsigned char  Hour;
    unsigned char  Minute;
    unsigned char  Second;
    unsigned char  Pad1;
    unsigned int   Nanosecond;
    int            TimeZone;
    unsigned char  Daylight;
    unsigned char  Pad2[3];
};
// Define the Reset Types
enum EFI_RESET_TYPE {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
};

// Define the Runtime Services struct
struct EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER Hdr;
    
    // Time Services
    long (*GetTime)(EFI_TIME* Time, void* Capabilities);
    long (*SetTime)(EFI_TIME* Time);
    long (*GetWakeupTime)(BOOLEAN* Enabled, BOOLEAN* Pending, EFI_TIME* Time);
    long (*SetWakeupTime)(BOOLEAN Enable, EFI_TIME* Time);

    // Virtual Memory Services
    void* SetVirtualAddressMap;
    void* ConvertPointer;

    // Variable Services
    void* GetVariable;
    void* GetNextVariableName;
    void* SetVariable;
    
    // Miscellaneous
    void* GetNextHighMonotonicCount;
    void (*ResetSystem)(
        EFI_RESET_TYPE ResetType,
        long ResetStatus,
        unsigned long DataSize,
        void* ResetData
    );
};

// --- EFI_SYSTEM_TABLE ---
struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER Hdr;
    CHAR16* FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
    EFI_RUNTIME_SERVICES* RuntimeServices;
    EFI_BOOT_SERVICES* BootServices;
    UINTN NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE* ConfigurationTable;
};

// --- Simple Text Output Mode ---
struct SIMPLE_TEXT_OUTPUT_MODE {
    INT64  MaxMode;
    INT64  Mode;
    INT64  Attribute;
    INT64  CursorColumn;
    INT64  CursorRow;
    BOOLEAN CursorVisible;
};

// --- Input protocol ---
struct EFI_INPUT_KEY {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
};

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET) (EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This, EFI_INPUT_KEY* Key);

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    VOID* WaitForKey;
};

// --- Output protocol function types ---
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, const CHAR16* String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, const CHAR16* String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, UINTN ModeNumber, UINTN* Columns, UINTN* Rows);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, UINTN ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, UINTN Attribute);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, UINTN Column, UINTN Row);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, BOOLEAN Visible);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET Reset;
    EFI_TEXT_STRING OutputString;
    EFI_TEXT_TEST_STRING TestString;
    EFI_TEXT_QUERY_MODE QueryMode;
    EFI_TEXT_SET_MODE SetMode;
    EFI_TEXT_SET_ATTRIBUTE SetAttribute;
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE* Mode;
};

// --- MSVC CRT stub ---
extern "C" int _fltused = 0;

// --- Helper Functions ---
EFI_INPUT_KEY Waitforkey(EFI_SYSTEM_TABLE* SystemTable) {
    EFI_INPUT_KEY key;
    while (true) {
        if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == EFI_SUCCESS) {
            return key;
        }
        __asm__ volatile("pause");
    }
}

void write(const CHAR16* string, EFI_SYSTEM_TABLE* SystemTable) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)string);
}

void clear(EFI_SYSTEM_TABLE* SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

void change_color(UINTN color, EFI_SYSTEM_TABLE* SystemTable) {
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, color);
}
void shutdown(EFI_SYSTEM_TABLE* SystemTable) {
    // Assuming 'SystemTable' is your instance of EFI_SYSTEM_TABLE
SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}
void reset (EFI_SYSTEM_TABLE* SystemTable) {
    // This triggers a full power-cycle (cold reboot)
SystemTable->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}

void PrintNumber(unsigned long long Number, EFI_SYSTEM_TABLE* SystemTable) {
    if (Number == 0) {
        write((CHAR16*)u"0", SystemTable);
        return;
    }

    CHAR16 buffer[20];
    int i = 0;
    
    // Extract digits
    while (Number > 0) {
        buffer[i++] = (Number % 10) + '0'; // Convert digit to char
        Number /= 10;
    }

    // Print in reverse
    while (i > 0) {
        CHAR16 digit[2] = { buffer[--i], 0 };
        write(digit, SystemTable);
    }
}

bool StringsEqual(const CHAR16* str1, const CHAR16* str2) {
    while (*str1 != 0 && *str2 != 0) {
        if (*str1 != *str2) return false;
        str1++;
        str2++;
    }
    return *str1 == *str2;
}

// --- Main Entry ---
extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable || !SystemTable->ConOut) return EFI_DEVICE_ERROR;

    clear(SystemTable);
    change_color(GREEN, SystemTable);
    write((CHAR16*)u"=========================================================\r\n", SystemTable);
    write((CHAR16*)u" Zenith26\r\n", SystemTable);
    write((CHAR16*)u"=========================================================\r\n\n\n", SystemTable);
    write((CHAR16*) u"Write Help if you dont know commands and etc...\r\n",SystemTable);
    change_color(WHITE, SystemTable);

    CHAR16 inputBuffer[128];
    UINTN bufferIndex = 0;

    while (true) {
        write((CHAR16*)u"> ", SystemTable);
        bufferIndex = 0;

        while (true) {
            EFI_INPUT_KEY key = Waitforkey(SystemTable);

            if (key.UnicodeChar == u'\r') { // Enter
                inputBuffer[bufferIndex] = 0;
                write((CHAR16*)u"\r\n", SystemTable);
                if (StringsEqual(inputBuffer, (CHAR16*)u"Help")) {
                    change_color(GREEN, SystemTable);
                    write((CHAR16*) u"What do you need help with?\r\n",SystemTable);
                    write((CHAR16*) u"Commands: for commands.\r\n",SystemTable);
                    change_color(WHITE, SystemTable);
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"commands")) {
                   change_color(GREEN, SystemTable);
                    write((CHAR16*) u"commands, version, clear screen, about, credits\r\n",SystemTable);
                    write((CHAR16*) u"reset, shut down, date, time.\r\n",SystemTable);
                    change_color(WHITE, SystemTable); 
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"clear screen")) {
                    clear(SystemTable);
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"version")) {
                   change_color(GREEN, SystemTable);
                    write((CHAR16*) u"Zenith v26.0.1(BETA)\r\n",SystemTable);
                    write((CHAR16*) u"Zenith2026 Copyright StarD\r\n",SystemTable);
                    change_color(WHITE, SystemTable); 
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"about")) {
                    change_color(GREEN, SystemTable);
                    write((CHAR16*) u"Zenith26\r\n",SystemTable);
                    write((CHAR16*) u"Zenith26v0.1 is a Text OS version (BETA)\r\n",SystemTable);
                    write((CHAR16*) u"This OS is UEFI OS\r\n",SystemTable);
                    write((CHAR16*) u"THANK YOU FOR TRYING!!!!\r\n",SystemTable);
                    change_color(WHITE, SystemTable);
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"credits")) {
                    change_color(GREEN, SystemTable);
                    write((CHAR16*) u"@stardtheprogrammer on Youtube, derbeargames on Github\r\n", SystemTable);
                    change_color(WHITE, SystemTable);
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"reset")) {
                    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, NULL);
                    while(1);
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"date")) {
                   EFI_TIME CurrentTime;
                   long Status = SystemTable->RuntimeServices->GetTime(&CurrentTime, NULL);

                   if (Status == 0) { // 0 usually represents EFI_SUCCESS
                   // Now CurrentTime.Year, CurrentTime.Month, etc., are populated!
                   PrintNumber(CurrentTime.Day, SystemTable);
                   write((CHAR16*) u"/", SystemTable);
                   PrintNumber( CurrentTime.Month, SystemTable);
                   write((CHAR16*) u"/", SystemTable);
                   PrintNumber( CurrentTime.Year, SystemTable);
                   write((CHAR16*) u"\r\n", SystemTable);
                   } 
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"time")) {
                   EFI_TIME CurrentTime;
                   long Status = SystemTable->RuntimeServices->GetTime(&CurrentTime, NULL);

                   if (Status == 0) { // 0 usually represents EFI_SUCCESS
                   // Now CurrentTime.Year, CurrentTime.Month, etc., are populated!
                   PrintNumber(CurrentTime.Hour, SystemTable);
                   write((CHAR16*) u":", SystemTable);
                   PrintNumber(CurrentTime.Minute, SystemTable);
                   write((CHAR16*) u":", SystemTable);
                   PrintNumber(CurrentTime.Second, SystemTable);
                   write((CHAR16*) u"\r\n", SystemTable);
                   } 
                } else if (StringsEqual(inputBuffer, (CHAR16*)u"shut down")) {
                    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, 0, 0, NULL);
                    while(1);
                } else {
                   change_color(RED, SystemTable); //red
                   write( (CHAR16*)u"\r\nError: Unknown Function!\r\n", SystemTable); 
                   change_color(WHITE, SystemTable);
                }
                break;
            } else if (key.UnicodeChar == u'\b') { // Backspace
                if (bufferIndex > 0) {
                    bufferIndex--;
                    write((CHAR16*)u"\b \b", SystemTable);
                }
            } else if (key.UnicodeChar != 0 && bufferIndex < 127) {
                inputBuffer[bufferIndex++] = key.UnicodeChar;
                CHAR16 echo[2] = { key.UnicodeChar, 0 };
                write((CHAR16*)echo, SystemTable);
            }
        }
    }
    return EFI_SUCCESS;
}