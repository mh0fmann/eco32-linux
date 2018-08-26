OUTPUT_ARCH(eco32)
ENTRY(start)


SECTIONS
{
	. = 0xC0400000;

	.text : {
		_stext = .;
		_etext = .;
	}
}
