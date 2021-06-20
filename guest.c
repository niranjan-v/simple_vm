#include <stddef.h>
#include <stdint.h>

#define O_MASK 0x8000
#define O_OPEN (O_MASK | 0)
#define O_CLOSE (O_MASK | 1)
#define O_READ (O_MASK | 2)
#define O_WRITE (O_MASK | 3)
#define O_SEEK (O_MASK | 4)
#define BS_MASK 0xE000
#define OUT_B  (BS_MASK | 0)
#define OUT_S (BS_MASK | 1)
#define OUT_P (BS_MASK | 2)
#define OUT_X (BS_MASK | 3)
#define UNUSED_VAR 0xdeadffffu

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x0040
#define O_APPEND 0x0400
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct kbuf{
	uint32_t fd;
	char *addr;
	uint32_t nbytes;
	uint32_t rbytes;
} kbuf;

typedef struct kseek{
	uint32_t fd;
	int offset;
	uint32_t whence;
	uint32_t roffset;
} kseek;

typedef struct kopen{
	char* fname;
	int flags;
	int mode;
	int fd;
} kopen;

static void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static inline void outs(uint16_t port, uint32_t value) {
  asm("out %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}


static inline uint32_t inb(uint16_t port) {
  uint8_t ret;
  asm("in %1, %0" : "=a"(ret) : "Nd"(port) : "memory" );
  return ret;
}


void printVal(uint32_t value) {
	outs(OUT_S, value); 
}

void display(const char * x){
	size_t i=0;
  	uint32_t addr = *((uint32_t *)&x + i);
  	outs(OUT_P, addr);
}

int open(char *fname, int flags, int mode){
	kopen ko,*ko_p;
	ko.fname=fname; ko.flags=flags; ko.fd=-1; ko.mode=mode;
	ko_p=&ko;
	size_t i=0; 
  	uint32_t addr = *((uint32_t *)&ko_p + i);
	outs(O_OPEN, addr);
	return ko.fd;
}

uint32_t close(uint32_t fd){
	uint32_t ret=-1;
	outs(O_CLOSE, fd);
	ret=inb(O_CLOSE);
	return ret;
}

uint32_t read(uint32_t fd, char *str, uint32_t nbytes){
	uint32_t ret=0;
	kbuf buf, *buf1;
	buf.fd=fd; buf.addr=str; 
	buf.nbytes=nbytes; buf.rbytes=0;
	size_t i=0; 
	buf1=&buf;
  	uint32_t addr = *((uint32_t *)&buf1 + i);
  	outs(O_READ,addr);
  	ret=buf.rbytes;
  	return ret;
}

uint32_t write(uint32_t fd, char *str, uint32_t nbytes){
	uint32_t ret=0;
	kbuf buf, *buf1;
	buf.fd=fd; buf.addr=str; 
	buf.nbytes=nbytes; buf.rbytes=0;
	size_t i=0; 
	buf1=&buf;
  	uint32_t addr = *((uint32_t *)&buf1 + i);
  	outs(O_WRITE,addr);
  	ret=buf.rbytes;
  	return ret;
}

uint32_t lseek(uint32_t fd, uint32_t offset, uint32_t whence){
	uint32_t ret=0;
	kseek ks, *ksp;
	ks.fd=fd; ks.offset=offset; ks.whence=whence; ks.roffset=0; 
	size_t i=0; 
	ksp = &ks;
  	uint32_t addr = *((uint32_t *)&ksp + i);
  	outs(O_SEEK,addr);
  	ret=ks.roffset ;
  	return ret;
}

void readn_display(char* fname){
	int fd=open(fname, O_RDWR, 0700);
	if (fd<0) {
		display("error couldn't open file\n");
		return;
	}
	///-----
	char str[129]={0}; 
	uint32_t nbytes=1, tbytes=0;
	while((nbytes=read(fd,str,128))>0){
		display(str); 
		tbytes+=nbytes;
		for(char* p=str;*p;++p){
			*p='\0';
		}
	}
	display("\n# bytes read : ");
	printVal(tbytes);
	///-----
	close(fd);
}

uint32_t getNumExits(){
	uint32_t ret=0, *ret_a;
	ret_a=&ret;
	size_t i=0; 
  	uint32_t addr = *((uint32_t *)&ret_a + i);
  	outs(OUT_X,addr);
  	return	ret;
}


void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
	const char *p;
	uint32_t numExits = getNumExits();
	printVal(numExits);
	for (p = "Hello, world!\n"; *p; ++p)
		outb(OUT_B, *p);
	numExits = getNumExits();
	printVal(numExits);
	display("Hello, world!\n");
	numExits = getNumExits();
	printVal(numExits);
	
	char* data1 ="!-----------------------64 Bytes of Data----------------------!\n";
	char* data2 ="!***********************64 Bytes of Data**********************!\n";

	uint32_t fd=open("file.txt",O_RDWR|O_CREAT,0700), wbytes=0;
	////------------write to file.txt
	for(int i=0;i<5;i++){
		lseek(fd,64,SEEK_CUR);
		wbytes+=write(fd,data1,64);
	}
	lseek(fd,-10*64,SEEK_CUR);
	for(int i=0;i<5;i++){
		wbytes+=write(fd,data2,64);
		lseek(fd,64,SEEK_CUR);
	}
	display("# bytes written : ");
	printVal(wbytes);
	///------------read from file.txt
	lseek(fd,0,SEEK_SET);
	char str[129]={0}; 
	uint32_t nbytes=1, tbytes=0;
	while((nbytes=read(fd,str,128))>0){
		display(str); 
		tbytes+=nbytes;
		for(char* p=str;*p;++p){
			*p='\0';
		}
	}
	display("\n# bytes read : ");
	printVal(tbytes);
	///-----
	// readn_display("guest.c"); //supports multiple files
	close(fd);
	///-------------------------------------------------------------------------//
	*(long *) 0x400 = 42;
	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}
