unpack_lznt1
============

A simple command-line Windows tool to uncompress Microsoft LZNT1-compressed files by calling [RtlDecompressBuffer](http://msdn.microsoft.com/en-us/library/windows/hardware/ff552191(v=vs.85).aspx).

Can skip bytes at the beginning/end of the compressed input.

Known limitations:
* Works on only small files, since it reads the whole file into memory.

