```-------------------------------------------------------------------------------
-----------H----H---X----X----CCCCC----22222-----0000-----0000-----11----------
----------H----H-----X-X----C--------------2---0----0---0----0---1-1-----------
---------HHHHHH------X-----C----------22222---0----0---0----0-----1------------
--------H----H-----X--X---C----------2-------0----0---0----0-----1-------------
-------H----H----X-----X--CCCCC-----222222---0000-----0000----11111------------
-------------------------------------------------------------------------------
HxC Floppy Emulator project - https://hxc2001.com/
(c) 2019-2022 Jean-François DEL NERO / (c) HxC2001
-------------------------------------------------------------------------------```

HxC Floppy Emulator QuickDisk Toolkit

HxC Floppy Emulator Roland, Akai and MO5 QuickDisk file image check and conversion tool.

Usage :
 
Check and convert to wave and raw bin files a Roland QD image (Roland S-10 / S-220 / ...) 
    
```qdhxcfe -checkrolandqd:'Roland_QD_File.QD'```
    
Check and convert to wave and raw bin file an Akai QD image (Akai MD-280(S612) / 700 / X3700 ...) 
    
```qdhxcfe -checkakaiqd:'Akai_QD_File.QD'```

Check and convert to raw bin file an Thomson MO5 QD image 

```qdhxcfe -checkmo5qd:'MO5_QD_File.QD'```

Convert a raw MFM BIN file to QD file with some specific timing parameters 
     
```qdhxcfe -qdstartsw:1500 -qdtrklen:7000 -qdreadylen:4600 -qddatadelay:200  -loadraw:INPUT.BIN -generate -foutput:OUTPUT.QD```

Convert a MO5 image file to QD file 

```qdhxcfe -loadmo5qd:INPUT.BIN -generate -foutput:OUTPUT.QD```

Syntax :

```
HxC Floppy Emulator : QD Floppy image file tool v0.0.2.2
Copyright (C) 2006-2022 Jean-Francois DEL NERO
This program comes with ABSOLUTELY NO WARRANTY
This is free software, and you are welcome to redistribute it
under certain conditions;

Options:
  -qdtrklen 			: QD track length (ms)
  -qdstartsw 			: QD start switch position (ms)
  -qdreadylen 			: QD ready length
  -qddatadelay 			: QD ready to data delay
  -loadraw 			    : Load a raw mfm track
  -loadmo5qd 			: Load Thomson MO5 QD file
  -generate 			: Generate a HxC QD file
  -checkmo5qd 			: Test a MO5 formatted HxC QD file
  -checkrolandqd 		: Test a Roland formatted HxC QD file
  -checkakaiqd 			: Test a Akai formatted HxC QD file
  -help 			    : This help
```

