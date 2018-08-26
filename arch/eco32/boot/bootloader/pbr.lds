OUTPUT_ARCH(eco32)
ENTRY(start)


SECTIONS
{
	. = 0xC0010000;

	.text : {
		_stext = .;
		_etext = .;
	}
	
	. = 0xC00101FE;
	
	.data : {
		_sdata = .;
		_edata = .;
	}
}
