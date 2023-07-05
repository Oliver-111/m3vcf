# vcflib 
The main functionality of the vcflib library is to efficiently read VCF files into memory. It supports both uncompressed VCF files and gzipped files compressed with zlib.  
**Compiling**  
Go to the directory of vcflib and compile vcflib
```Bash
cd zM3vcf/vcflib  
make
```
**Demo**  
The vcflib demo codes is the zM3vcf/vcflib/tst/simpleRead.c file. Go to the directory and compile demo (current work path is zM3vcf/vcflib）  
```Bash
cd tst  
make simpleRead   
```
The demo demonstrates the simplest read operation.
# zM3vcf
The main functionality of the zM3vcf library is to efficiently compress vcf to m3vcf, and convert m3vcf to vcf. It supports both uncompressed VCF files and gzipped files compressed with zlib.  
**Compiling**  
Go to the directory of zM3vcf, make sure compile vcflib first, then compile and generate the zM3vcf tool and the m3vcf interface testing program(m3vcfTest)
```Bash
cd zM3vcf  
make vcflib  
make  
```
zM3vcf:		command line tools.  
m3vcfTest: 	m3vcf interface testing program.  
For detail, please refer to FM3VCF_instruction.pdf.
