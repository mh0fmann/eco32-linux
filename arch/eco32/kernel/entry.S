/*
 * Linux architectural port borrowing liberally from similar works of
 * others, namely OpenRISC and RISC-V.  All original copyrights apply
 * as per the original source declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <asm/mvtfs.h>
#include <asm/ptrace.h>
#include <asm/page.h>
#include <asm/thread_info.h>
#include <asm/asm-offsets.h>
#include <asm/irq.h>

#include <linux/linkage.h>


#define DISABLE_INTERRUPTS(r)    \
    mvfs    r,PSW               ;\
    andi    r,r,~PSW_CIE        ;\
    mvts    r,PSW

#define ENABLE_INTERRUPTS(r)     \
    mvfs    r,PSW               ;\
    ori     r,r,PSW_CIE         ;\
    mvts    r,PSW

#define SAVE_REGISTERS          \
    stw     $0,$29,r0_OFF       ;\
    stw     $1,$29,r1_OFF       ;\
    stw     $2,$29,r2_OFF       ;\
    stw     $3,$29,r3_OFF       ;\
    stw     $4,$29,r4_OFF       ;\
    stw     $5,$29,r5_OFF       ;\
    stw     $6,$29,r6_OFF       ;\
    stw     $7,$29,r7_OFF       ;\
    stw     $8,$29,r8_OFF       ;\
    stw     $9,$29,r9_OFF       ;\
    stw     $10,$29,r10_OFF     ;\
    stw     $11,$29,r11_OFF     ;\
    stw     $12,$29,r12_OFF     ;\
    stw     $13,$29,r13_OFF     ;\
    stw     $14,$29,r14_OFF     ;\
    stw     $15,$29,r15_OFF     ;\
    stw     $16,$29,r16_OFF     ;\
    stw     $17,$29,r17_OFF     ;\
    stw     $18,$29,r18_OFF     ;\
    stw     $19,$29,r19_OFF     ;\
    stw     $20,$29,r20_OFF     ;\
    stw     $21,$29,r21_OFF     ;\
    stw     $22,$29,r22_OFF     ;\
    stw     $23,$29,r23_OFF     ;\
    stw     $24,$29,r24_OFF     ;\
    stw     $25,$29,r25_OFF     ;\
    stw     $26,$29,r26_OFF     ;\
    stw     $27,$29,r27_OFF     ;\
    stw     $28,$29,r28_OFF     ;\
    stw     $25,$29,r29_OFF     ;\
    stw     $30,$29,r30_OFF     ;\
    stw     $31,$29,r31_OFF     ;\
    stw     $2,$29,r2_orig_OFF  ;\
    mvfs    $24,PSW             ;\
    stw     $24,$29,psw_OFF

#define RESTORE_REGISTERS        \
    ldw     $1,$29,r1_OFF       ;\
    ldw     $2,$29,r2_OFF       ;\
    ldw     $3,$29,r3_OFF       ;\
    ldw     $4,$29,r4_OFF       ;\
    ldw     $5,$29,r5_OFF       ;\
    ldw     $6,$29,r6_OFF       ;\
    ldw     $7,$29,r7_OFF       ;\
    ldw     $8,$29,r8_OFF       ;\
    ldw     $9,$29,r9_OFF       ;\
    ldw     $10,$29,r10_OFF     ;\
    ldw     $11,$29,r11_OFF     ;\
    ldw     $12,$29,r12_OFF     ;\
    ldw     $13,$29,r13_OFF     ;\
    ldw     $14,$29,r14_OFF     ;\
    ldw     $15,$29,r15_OFF     ;\
    ldw     $16,$29,r16_OFF     ;\
    ldw     $17,$29,r17_OFF     ;\
    ldw     $18,$29,r18_OFF     ;\
    ldw     $19,$29,r19_OFF     ;\
    ldw     $20,$29,r20_OFF     ;\
    ldw     $21,$29,r21_OFF     ;\
    ldw     $22,$29,r22_OFF     ;\
    ldw     $23,$29,r23_OFF     ;\
    ldw     $24,$29,r24_OFF     ;\
    ldw     $25,$29,r25_OFF     ;\
    ldw     $26,$29,r26_OFF     ;\
    ldw     $27,$29,r27_OFF     ;\
    ldw     $28,$29,r28_OFF     ;\
    ldw     $30,$29,r30_OFF     ;\
    ldw     $31,$29,r31_OFF     ;\
    ldw     $24,$29,psw_OFF     ;\
    .nosyn                      ;\
    ldhi    $25,0xFFFF0000      ;\
    and     $24,$24,$25         ;\
    ldhi    $25,irqmask         ;\
    ori     $25,$25,irqmask     ;\
    ldw     $25,$25,0           ;\
    or      $24,$24,$25         ;\
    .syn                        ;\
    mvts    $24,PSW             ;\
    ldw     $29,$29,r29_OFF


    .global isr_tbl

    .global irqmask
    .global current_ti
    .global def_xcpt_handler

    .text

/*
 * common kernel entry
 * all interrupts and faults arrive here
 * (with a single exception: user TLB misses go to 'umiss' below)
 */
ENTRY(xcept)
    .nosyn
    mvfs    $24,PSW                 # get PSW
    slri    $24,$24,16              # extract exception identifier
    andi    $24,$24,0x1F
    subi    $24,$24,XCPT_TLB_MISS   # is it a kernel TLB miss ?
    beq     $24,$0,umiss            # this is also handled there
page_fault:                         # page faults are handled here, too
    add     $25,$0,$29              # save original sp in $25
    mvfs    $24,PSW                 # get PSW
    slri    $24,$24,25              # shift previous mode to bit 0
    andi    $24,$24,1               # check it
    beq     $24,$0,from_kernel      # which mode did we come from?
from_user:                          # switch to kernel stack
    ldhi    $24,current_ti
    ori     $24,$24,current_ti
    ldw     $24,$24,0
    addi    $29,$24,THREAD_SIZE     # kernel stack is at the end of it
from_kernel:                        # valid kernel stack (now) present
    subi    $29,$29,PT_SIZE         # make room for pt_regs
    SAVE_REGISTERS
    .syn
    ldw     $27,$0,current_ti       # now load current ti in $27
    addi    $5,$29,0                # second arg: ptr to pt_regs
    mvfs    $4,PSW                  # first arg: IRQ number
    slri    $4,$4,16
    andi    $4,$4,0x1F
    ori     $9,$0,NR_IRQS
    bgeu    $9,$4,call_isr          # interrupt or exception?
    ENABLE_INTERRUPTS($8)           # exceptions run with interrupts enabled
call_isr:
    slli    $9,$4,2                 # compute offset into ISR table
    ldw     $9,$9,isr_tbl           # get adress of service routine
    jalr    $9                      # call service routine
    j       rfxcept
END(xcept)

/*
 * common kernel exit
 * after handling of interrupts and exceptions we arrive here
 */
ENTRY(rfxcept)
    ldw     $4,$29,psw_OFF          # must check for work before we leave
    andi    $4,$4,PSW_PUM           # are we returning to kernel mode ?
    beq     $4,$0,no_more_work      # yes - then don't check for work
check_work:
    DISABLE_INTERRUPTS($8)
    ldw     $5,$27,TI_FLAGS         # second arg: thread info flags
    andi    $4,$5,_TIF_WORK_MASK    # any work to do ?
    beq     $4,$0,no_more_work      # no - then leave
    addi    $4,$29,0                # first arg: ptr to pt_regs
    jal     do_work_pending         # now do the work
    j       check_work              # check for more work
no_more_work:
    DISABLE_INTERRUPTS($8)
    RESTORE_REGISTERS
    rfx
END(rfxcept)


/*
 * user TLB misses arrive here
 */
ENTRY(umiss)
    .nosyn
    mvfs    $25,TLB_ENTRY_HI        # get missing page addr
    ldhi    $26,current_pgd         # get base addr of page dir
    ori     $26,$26,current_pgd
    ldw     $26,$26,0
    add     $24,$0,$25              # missing page addr
    slri    $24,$24,22              # first level index
    slli    $24,$24,2               # first level offset
    add     $26,$26,$24             # add to page dir base addr
    ldw     $26,$26,0               # load second level page table
    beq     $26,$0,page_fault       # NULL - page fault
    slri    $26,$26,12              # set rightmost 12 bits to zero
    slli    $26,$26,12
    add     $24,$0,$25              # missing page addr
    slli    $24,$24,10              # second level index
    slri    $24,$24,22
    slli    $24,$24,2               # second level offset
    add     $26,$26,$24             # add to page table base addr
    ldw     $26,$26,0               # load page table element
    andi    $24,$26,1               # present bit set?
    beq     $24,$0,page_fault       # no - page fault
    mvts    $26,TLB_ENTRY_LO        # else write random to TLB
    mvts    $25,TLB_ENTRY_HI
    tbwr
    rfx
    .syn
END(umiss)


/*
 * ISR table
 *
 * This table holds the addresses of the service routines for the
 * different interrupts and exceptions.
 */
    .data
    .align  4
isr_tbl:
    /* 00 - 15 : interrupts */
    .word   def_xcpt_handler    # 00: terminal 0 transmitter interrupt
    .word   def_xcpt_handler    # 01: terminal 0 receiver interrupt
    .word   def_xcpt_handler    # 02: terminal 1 transmitter interrupt
    .word   def_xcpt_handler    # 03: terminal 1 receiver interrupt
    .word   def_xcpt_handler    # 04: keyboard interrupt
    .word   def_xcpt_handler    # 05: unused
    .word   def_xcpt_handler    # 06: unused
    .word   def_xcpt_handler    # 07: unused
    .word   def_xcpt_handler    # 08: disk interrupt
    .word   def_xcpt_handler    # 09: unused
    .word   def_xcpt_handler    # 10: unused
    .word   def_xcpt_handler    # 11: unused
    .word   def_xcpt_handler    # 12: unused
    .word   def_xcpt_handler    # 13: unused
    .word   def_xcpt_handler    # 14: timer 0 interrupt
    .word   def_xcpt_handler    # 15: timer 1 interrupt
    /* 16 - 31 : exceptions */
    .word   def_xcpt_handler    # 16: bus timeout exception
    .word   def_xcpt_handler    # 17: illegal instruction exception
    .word   def_xcpt_handler    # 18: privileged instruction exception
    .word   def_xcpt_handler    # 19: divide instruction exception
    .word   def_xcpt_handler    # 20: trap instruction exception
    .word   def_xcpt_handler    # 21: TLB miss exception
    .word   def_xcpt_handler    # 22: TLB write exception
    .word   def_xcpt_handler    # 23: TLB invalid exception
    .word   def_xcpt_handler    # 24: illegal address exception
    .word   def_xcpt_handler    # 25: privileged address exception
    .word   def_xcpt_handler    # 26: unused
    .word   def_xcpt_handler    # 27: unused
    .word   def_xcpt_handler    # 28: unused
    .word   def_xcpt_handler    # 29: unused
    .word   def_xcpt_handler    # 30: unused
    .word   def_xcpt_handler    # 31: unused
