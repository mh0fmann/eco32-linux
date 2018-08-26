#
# mvstrt.s -- move the kernel down to 0xC0000000 and start it
#

.global start

.section .text

start:
	addi	$8,$0,linux
	addi	$9,$0,0xC0000000
	addi	$10,$9,0x00400000-256
loop:
	ldw		$11,$8,0			# copy word
	stw		$11,$9,0
	addi	$8,$8,4				# bump pointers
	addi	$9,$9,4
	bltu	$9,$10,loop			# more?
	cctl	7					# sync caches
	addi	$8,$0,0xC0000000	# start execution
	jr		$8

	# kernel comes after this
.align 4
linux:
