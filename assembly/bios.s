@ BIOS wrappers that C cannot express.
	.syntax unified
	.thumb
	.section .text

	.global BitUnPack
	.thumb_func
BitUnPack:
	swi 0x10
	bx lr

@ void CpuFill32(u32 value, void *dest, u32 bytes)  - BIOS CpuSet, 32-bit fill (safe for VRAM/OAM)
	.global CpuFill32
	.thumb_func
CpuFill32:
	sub sp, #4
	str r0, [sp]
	mov r0, sp
	lsrs r2, r2, #2
	movs r3, #1
	lsls r3, r3, #24
	orrs r2, r3
	lsls r3, r3, #2
	orrs r2, r3
	swi 0x0B
	add sp, #4
	bx lr
