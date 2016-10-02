%define USERSPACE_CODE_SEGEMENT_ENTRY (0x18 | 3)
%define USERSPACE_DATA_SEGEMENT_ENTRY (0x20 | 3)

global jmp_to_userspace

section .text

; Args:
; ebp + 8 -> address to jmp to(eip)
; ebp + 16 -> the user space stack.
jmp_to_userspace:
    mov ax, USERSPACE_DATA_SEGEMENT_ENTRY
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push USERSPACE_DATA_SEGEMENT_ENTRY
    push dword [ebp + 16]
    pushf
    push USERSPACE_CODE_SEGEMENT_ENTRY
    push dword [ebp + 8]
    iret
