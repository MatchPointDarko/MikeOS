%define USERSPACE_CODE_SEGEMENT_ENTRY (0x18 | 3)
%define USERSPACE_DATA_SEGEMENT_ENTRY (0x20 | 3)

global jmp_to_userspace

section .text

; Args:
; ebp + 8 -> dword, address to jmp to(eip)
; ebp + 16 -> dword, the user space stack.
jmp_to_userspace:
    mov ebp, esp ; No need to save ebp on the stack
    mov ax, USERSPACE_DATA_SEGEMENT_ENTRY
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push USERSPACE_DATA_SEGEMENT_ENTRY
    mov eax, [ebp + 8]
    push eax
    pushf
    push USERSPACE_CODE_SEGEMENT_ENTRY
    mov eax, [ebp + 4]
    push eax
    iret
