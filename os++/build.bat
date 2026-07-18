@echo off
cls
echo =========================================
echo       BUILDING Zenith OS
echo =========================================

:: 1. Auto-create the directory structure if it doesn't exist
if not exist "esp\EFI\BOOT" (
    echo Creating EFI directory structure...
    mkdir "esp\EFI\BOOT"
)

:: 2. Compile C++ with standard bare-metal flags
echo [1/3] Compiling C++ code...
"C:\Program Files\LLVM\bin\clang++" -target x86_64-pc-win32-coff -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fshort-wchar -mno-red-zone -c os.cpp -o os.o
if %errorlevel% neq 0 goto error

:: 3. Link with explicit, ultra-strict UEFI alignment
echo [2/3] Linking into EFI application...
"C:\Program Files\LLVM\bin\lld-link" /subsystem:efi_application /entry:efi_main /dll /nodefaultlib /driver /ALIGN:4096 /out:esp\EFI\BOOT\BOOTX64.EFI os.o
if %errorlevel% neq 0 goto error

:: 4. Clean up the object file
del os.o

:: 5. Run in QEMU
echo [3/3] Launching QEMU...
"C:\Program Files\qemu\qemu-system-x86_64" -bios OVMF.fd -drive format=raw,file=fat:rw:esp -vga std -net none
goto end

:error
echo.
echo !!!!!!! BUILD FAILED !!!!!!!
echo.
pause

:end