#
# pbr.s -- the partition boot record
#

# Runtime environment:
#
# This code must be loaded and started at 0xC0010000.
# It allocates a stack from 0xC0011000 downwards. So
# it must run within 4K (code + data + stack).
#
# This code expects the disk number of the boot disk
# in $16, the start sector of the disk or partition
# to be booted in $17 and its size in $18.
#
# The Linux binary image (together with the tiny "mvstrt"
# program), which is loaded by this code, must be in
# standalone (headerless) executable format, stored at
# partition relative disk sectors 1..8192, and gets
# loaded and started at 0xC0400000.


.global start

.section .text
	# load the image and start it
start:
	addi	$29,$0,0xC0011000	# setup stack
	addi	$4,$0,strtmsg		# say what is going on
	jal		msgout
	# $19 = sector number
	# $20 = load address
	# $21 = sectors left
	addi	$19,$0,1			# start loading with sector 1
	addi	$20,$0,0xC0400000	# where to load the image
	addi	$21,$0,8192			# how many sectors to load
start1:
	addi	$4,$0,'.'
	jal		chrout
	add		$4,$0,$19
	add		$5,$0,$20
	andi	$5,$5,0x3FFFFFFF
	addi	$6,$0,256			# load in pieces of 256 sectors
	jal		rdsct
	addi	$19,$19,256
	addi	$20,$20,256*512
	subi	$21,$21,256
	bgt		$21,$0,start1
	addi	$4,$0,mvmsg			# say what is going on
	jal		msgout
	cctl	7					# sync caches
	addi	$8,$0,0xC0400000	# start executing mvstrt
	jr	$8

	# read disk sectors
	#   $4 start sector number (disk or partition relative)
	#   $5 transfer address
	#   $6 number of sectors
rdsct:
	subi	$29,$29,32
	stw		$31,$29,20
	stw		$6,$29,16			# sector count
	add		$7,$5,$0			# transfer address
	add		$6,$4,$17			# relative sector -> absolute
	addi	$5,$0,'r'			# command
	add		$4,$0,$16			# disk number
	addi	$8,$0,0xC0000028
	jalr	$8
	bne		$2,$0,rderr			# error?
	ldw		$31,$29,20
	addi	$29,$29,32
	jr		$31

	# disk read error
rderr:
	addi	$4,$0,dremsg
	jal		msgout
	j		halt

	# output message
	#   $4 pointer to string
msgout:
	subi	$29,$29,8
	stw		$31,$29,4
	stw		$16,$29,0
	addi	$16,$4,0			# $16: pointer to string
msgout1:
	ldbu	$4,$16,0			# get character
	beq		$4,$0,msgout2		# done?
	jal		chrout				# output character
	addi	$16,$16,1			# bump pointer
	j		msgout1				# continue
msgout2:
	ldw		$16,$29,0
	ldw		$31,$29,4
	addi	$29,$29,8
	jr		$31

	# output character
	#   $4 character
chrout:
	subi	$29,$29,4
	stw		$31,$29,0
	addi	$8,$0,0xC0000020
	jalr	$8
	ldw		$31,$29,0
	addi	$29,$29,4
	jr		$31

	# halt execution by looping
halt:
	addi	$4,$0,hltmsg
	jal		msgout
halt1:
	j		halt1

	# messages
strtmsg:
	.string	"Loading image "
mvmsg:
	.string	"\r\nMoving image\r\n"
dremsg:
	.string	"Disk read error\r\n"
hltmsg:
	.string	"Bootstrap halted\r\n"

# boot record signature
.section .data
	.byte	0x55, 0xAA
