// ============================================================
//  Cryo OS - UEFI Bootloader
//  Bare-metal C++ targeting x86_64 UEFI
// ============================================================

// --- UEFI base types ---
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef UINT64              UINTN;
typedef INT64               INTN;
typedef UINT16              CHAR16;
typedef unsigned char       BOOLEAN;
typedef void                VOID;
typedef VOID*               EFI_HANDLE;
typedef UINTN               EFI_STATUS;

// --- Calling convention ---sss
#define EFIAPI __attribute__((ms_abi))

// --- Status codes ---
#define EFI_SUCCESS             0
#define EFI_DEVICE_ERROR        0x8000000000000007ULL

// --- Forward declarations ---
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct EFI_RUNTIME_SERVICES;
struct EFI_BOOT_SERVICES;
struct EFI_CONFIGURATION_TABLE;

// --- EFI_TABLE_HEADER (UEFI 2.10 §4.2) ---
struct EFI_TABLE_HEADER {
    UINT64  Signature;
    UINT32  Revision;
    UINT32  HeaderSize;
    UINT32  CRC32;
    UINT32  Reserved;
};

// --- EFI_SYSTEM_TABLE (UEFI 2.10 §4.3) ---
//  Every pointer is 8 bytes on x86_64; UINT32 fields carry
//  implicit 4-byte padding to the next 8-byte-aligned member.
struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;
    CHAR16*                         FirmwareVendor;
    UINT32                          FirmwareRevision;
    // 4 bytes implicit padding here on x86_64
    EFI_HANDLE                      ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL*  ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
    EFI_RUNTIME_SERVICES*           RuntimeServices;
    EFI_BOOT_SERVICES*              BootServices;
    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE*        ConfigurationTable;
};

// --- Simple Text Output Mode (UEFI 2.10 §12.4) ---
struct SIMPLE_TEXT_OUTPUT_MODE {
    INT64   MaxMode;
    INT64   Mode;
    INT64   Attribute;
    INT64   CursorColumn;
    INT64   CursorRow;
    BOOLEAN CursorVisible;
};

//input key for UEFI
struct EFI_INPUT_KEY {
  UINT16 scancodefromkey;
  CHAR16 UnicodeChar;
};
//the kinda class with functions like reset and when pressed
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
// Function pointer signatures for reading keys
typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET) (
EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This, 
BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(
EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
EFI_INPUT_KEY* Key);

// --- Function pointer types for Simple Text Output Protocol ---
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    BOOLEAN                          ExtendedVerification);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    const CHAR16*                    String);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    const CHAR16*                    String);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    UINTN                            ModeNumber,
    UINTN*                           Columns,
    UINTN*                           Rows);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    UINTN                            ModeNumber);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    UINTN                            Attribute);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    UINTN                            Column,
    UINTN                            Row);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    BOOLEAN                          Visible);

// --- EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL (UEFI 2.10 §12.4) ---
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET               Reset;
    EFI_TEXT_STRING              OutputString;
    EFI_TEXT_TEST_STRING         TestString;
    EFI_TEXT_QUERY_MODE          QueryMode;
    EFI_TEXT_SET_MODE            SetMode;
    EFI_TEXT_SET_ATTRIBUTE       SetAttribute;
    EFI_TEXT_CLEAR_SCREEN        ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR       EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE*     Mode;
};
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
  EFI_INPUT_RESET resetkey;
  EFI_INPUT_READ_KEY ReadKeyStroke;
  VOID* WaitForKeyCode;
};

// --- MSVC CRT stub (required by lld-link for floating-point) ---
extern "C" int _fltused = 0;

EFI_INPUT_KEY Waitforkey(EFI_SYSTEM_TABLE* SystemTable) {
    EFI_INPUT_KEY key;
    key.scancodefromkey = 0;      // Manual initialization
    key.UnicodeChar = 0;   // Manual initialization
    EFI_STATUS status;
    while (true) {
        // Query the keyboard buffer
        status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

        if(status == 0) {
            break;
        }
        __asm__ volatile("pause");
    }
    return key;
}
bool StringsEqual(const CHAR16* str1, const CHAR16* str2) {
    while (*str1 != 0 && *str2 != 0) {
        if (*str1 != *str2) return false;
        str1++;
        str2++;
    }
    return *str1 == *str2;
}

// ============================================================
//  ENTRY POINT
// ============================================================
extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle,
                                       EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable || !SystemTable->ConOut) {
        return EFI_DEVICE_ERROR;
    }

    // Clear screen
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    // Green text on black background  (EFI_GREEN = 0x0A)
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0A);

    // Print boot banner
    SystemTable->ConOut->OutputString(SystemTable->ConOut,
        (CHAR16*)u"====================================\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut,
        (CHAR16*)u"    GG OS BOOTED SUCCESSFULLY!   \r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut,
        (CHAR16*)u"====================================\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u" GG os basic lvl:\r \n");
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0F);
    SystemTable->ConOut->EnableCursor(SystemTable->ConOut, true);
        
    CHAR16 inputBuffer[128]; // Array to hold up to 127 characters + null terminator
    UINTN bufferIndex = 0;   // Keeps track of where to put the next character
    // Halt the CPU in an infinite loop
    while (true) {
    EFI_INPUT_KEY key = Waitforkey(SystemTable);

    if (key.UnicodeChar != 0) {
        // --- Handle Enter ---
        if (key.UnicodeChar == u'\r') {
            inputBuffer[bufferIndex] = 0; // Null-terminate

            if (StringsEqual(inputBuffer, (CHAR16*)u"Commands")) {
                SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0E); // Set to Yellow
             SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"\r\nCommands: Commands, Clearscr, Version\r\n");
             SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0f);//white
            } else if (StringsEqual(inputBuffer, (CHAR16*)u"Clearscr")){
            SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
            } else if (StringsEqual(inputBuffer, (CHAR16*)u"Version"))  {
            SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0E); // Set to Yellow
            SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"\r\nGG OS version: 0.1(BETA)\r\n");
            SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0F);//white
            } else {
                SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x04); //red
              SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"\r\nError: Unknown Function!\r\n"); 
              SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0f);//white
            }
            SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0A);//Green
            SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"> ");
            SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0F);//white
            bufferIndex = 0;
        }
        // --- Handle Backspace ---
        else if (key.UnicodeChar == u'\b') {
            if (bufferIndex > 0) {
                bufferIndex--;
                SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"\b \b");
            }
        }
        // --- Handle Normal Characters ---
        else if (bufferIndex < 127) {
            inputBuffer[bufferIndex] = key.UnicodeChar; // Add to buffer
            bufferIndex++;
            
            // Print the character to screen
            CHAR16 echo[2] = { key.UnicodeChar, 0 };
            SystemTable->ConOut->OutputString(SystemTable->ConOut, echo);
        }
    }
    
    }

    return EFI_SUCCESS;
}