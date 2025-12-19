# BE-Force-RWX-Regions
## Desc. Turning ROP into possible Shellcode execution through the Global Offset Table section (ft. ASLR Bypass). 
### Requirements: No PIE, No canary, Partial RELRO
## Thesis
Force RWX GOT section for shellcode execution by libc's built in procedure, mprotect.

## Intro
The PLT and GOT table are the basis to linking external libraries to your main binary on execution. System, printf, fgets, fputs, etc are libc procedures. These procedures aren't a part of the binary on compilation time, as each file that
uses libc or any other external libraries would be massive. The idea is to load in these external libraries on runtime in virtual memory. This saves a lot of storage space and also allows other programs to access the same libc libraries as well.
The PLT's job is to resolve the addresses to jump to when calling libc functions. The GOT section may contain the addresses of the libc functions already, saving a lot of the PLT's job. So whenever system is called, it first goes to the PLT to see if
the GOT already has this address resolved, and if it is it simply jumps to that address. Otherwise, the PLT itself resolves it through ld.so and puts it in the GOT.
<img width="921" height="190" alt="image" src="https://github.com/user-attachments/assets/03aa497d-26cc-4da8-9309-c791e3417610" />
In this example, we can see that the PLT resolution jumps to the GOT entry. If the GOT does not contain the address of the libc location of GETS yet, then it will jump back to 0x401046, and then continue execution from there, which is the PLT resolving the address.
After resolving, it will store it in the GOT. On first time execution of these external functions, they will always resolve through the PLT first and then inserted into the GOT.
## Practicality
ANYWAYS so what does that even have to do with anything? The idea is that if we can overwrite the GOT table with addresses we want and ALSO make the GOT executable, we get a win.
## Preparation
**The three libc functions we will be examining are GETS, PUTS, and MPROTECT. The argument architecture is listed below.\**
char \*gets(char \*s);\
int puts( const char\* str );\
int mprotect(void addr, size_t size, int prot);\
_Even though there is a very quick win through simply a pop rdi; ret gadget, leak of aslr address through puts, and then ret2system, we will be focusing on making sections RWX with mprotect.\_
**Our controlled flow of the program will be so:\**
pop rdi; ret gadget of puts got address\
call puts to leak aslr\
pop rdi; ret gadget of puts got address\
call GETS to write to puts got address --> we will be overwritting both puts and gets GOT address with the pop rdx gadget and mprotect locations\
pop rdi; ret gadget\
pop rsi; ret gadget\
pop rdx; ret gadget in libc\
call mprotect using aslr bypass leak\  
To summary, we're calling puts initially to leak the PUTS GOT entry to defeat ASLR\
Calling GETS to overwrite both PUTS and GETS GOT entry with the pop rdx, and mprotect addresses calculated by ASLR Base + Offset of pop rdx and mprotect procedures.\
Calling mprotect after prepping rdi, rsi, and rdx.\
## Architecutre of payload
The architecture of our payloads is such:\n
1st payload will control our stack, where it is such
pop rdi; ret\
[puts got address]\
call puts\
pop rdi; ret\
[puts got address]\
call gets\
pop rdi; ret\
[location to change perms]\
pop rsi; ret\
[how many bytes to change perms]
[dummy value due to the nature of the gadget ( you can find out more details after doing ROP hunting yourself )]\
pop rdx (oversimplified); ret\
[rwx as an int (7)]\
[dummy value for the same reasons above]\
[dummy value]\
[dummy value]\
[dummy value]\
[dummy value]\
call mprotect\
Our second payload after running gets will contain the following:\
Calculated addr gadget pop rdx + Calculated addr of mprotect.
