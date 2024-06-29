.global _start      
.align 4    

_start: 
    mov X0, #1
    adrp X1, helloworld@page
    add X1, X1, helloworld@pageoff 
    mov X2, #13    
    mov X16, #4
    svc #0x80

    mov     X0, #0      
    mov     X16, #1     
    svc     #0x80       

    .data
helloworld:      
    .ascii  "Hello World!\n"