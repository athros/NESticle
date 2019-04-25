                .486
                .MODEL  flat
                LOCALS

                .DATA
                EXTRN _PITCH:DWORD,_SCREENX:DWORD,_SCREENY:DWORD

IMG STRUC 
  type  dd 2
  size  dd ?
  xw    dd ?
  yw    dd ?
IMG ENDS


                .CODE
                PUBLIC  _drawimager2,_loadpalette8,_loadpalette6,_drawrect,_drawrectmap
                PUBLIC  _drawhline,_drawvline,_drawscr
;;QUAD PALETTES!!!!!

;sets an 6-bit palette
_loadpalette6   PROC    NEAR
                ARG     @@PAL:DWORD,@@FIRST:DWORD,@@CNT:DWORD

                push    ebp
                mov     ebp,esp
                push    esi

                mov     esi,@@PAL
                mov     eax,@@FIRST
                mov     ecx,@@CNT

                mov     edx,3C8h
                out     dx,al   ;Set first

                inc     dx

@@LP:           mov     al,[esi+2]
                out     dx,al
                mov     al,[esi+1]
                out     dx,al
                mov     al,[esi+0]
                out     dx,al

                add     esi,4
                dec     ecx
                jnz     @@LP


                pop     esi
                pop     ebp
                ret
_loadpalette6   ENDP

;sets a 8-bit palette (converting it to 6bit)
_loadpalette8   PROC    NEAR
                ARG     @@PAL:DWORD,@@FIRST:DWORD,@@CNT:DWORD

                push    ebp
                mov     ebp,esp
                push    esi

                mov     esi,@@PAL
                mov     eax,@@FIRST
                mov     ecx,@@CNT

                mov     edx,3C8h
                out     dx,al   ;Set first

                cld
                inc     dx

@@LP:           mov     al,[esi+2]
                shr     al,2
                out     dx,al
                mov     al,[esi+1]
                shr     al,2
                out     dx,al
                mov     al,[esi+0]
                shr     al,2
                out     dx,al

                add     esi,4
                dec     ecx
                jnz     @@LP

                pop     esi
                pop     ebp
                ret
_loadpalette8   ENDP


                

;void drawimager2(IMG *src,char *dest,int x,int y,int o)
_drawimager2    PROC    
        ARG @@SRC,@@DEST,@@X,@@Y,@@O
        
        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi
        push    edi
        
        mov     esi,[@@SRC]  ;source IMG
        mov     edi,[@@DEST] ;dest
        test    esi,esi
        jz      @@DONE

;do clip testing
        xor     eax,eax
        mov     ecx,@@X
        mov     edx,@@Y
        add     ecx,[esi.xw] ;right extent of image
        add     edx,[esi.yw] ;bottom extent of image
        
        
        cmp     [@@X],eax    ;left clip
        jl      drawimager2clip
        cmp     [_SCREENX],ecx ;right clip
        jl      drawimager2clip
        cmp     [@@Y],eax      ;top clip
        jl      drawimager2clip
        cmp     [_SCREENY],edx ;bottom clip
        jl      drawimager2clip

        ;point to memory for dest
        mov     eax,_PITCH
        mul     @@Y
        add     eax,@@X
        add     edi,eax

        ;test x flip
        test    [@@O],2
        jnz     @@XFLIP
        call    drawr2
        jmp     @@DONE
@@XFLIP:call    drawr2_xflip

@@DONE:
        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        ret
_drawimager2    ENDP


;image drawing prep function
;determines clipping values and calls clip core function
drawimager2clip PROC
        ARG @@SRC,@@DEST,@@X,@@Y,@@O
        xor     ebx,ebx

;top clip
        xor     eax,eax
        cmp     [@@Y],eax
        jg      @@NOTOP
        cmp     edx,eax ;top edge is <0
        jle     @@DONE

        sub     eax,@@Y ;ystart=0-Y
        mov     @@Y,ebx ;Y=0
@@NOTOP:  push  eax  ;store YSTART

;bottom clip
        mov     eax,[esi.yw]
        sub     edx,[_SCREENY]
        jle     @@NOBOTTOM

        sub     eax,edx ;ylen=SCREENY-Y
        jle     @@DONE
@@NOBOTTOM:
        sub     eax,[esp] ;ylen-=ystart
        push    eax  ;store YLEN


;left clip
        xor     eax,eax
        cmp     [@@X],eax
        jg      @@NOLEFT
        cmp     ecx,eax ;right edge is <0
        jle     @@DONE

        sub     eax,@@X   ;xstart=0-X
        mov     [@@X],ebx ;X=0

@@NOLEFT: push    eax ;store XSTART

;right clip
        mov     eax,[esi.xw]
        sub     ecx,[_SCREENX]
        jle     @@NORIGHT
        sub     eax,ecx
        jle     @@DONE
@@NORIGHT:
        sub     eax,[esp] ;xlen-=xstart
        push    eax  ;store XLEN

        push  @@O

        ;point to memory for dest
        mov     eax,_PITCH
        push    eax
        mul     @@Y
        add     eax,@@X
        add     edi,eax

        ;test x flip
        test    [@@O],2
        jnz     @@XFLIP
        call    drawr2_xyclip
        jmp     @@DONE
@@XFLIP:call    drawr2_xyclip_xflip


@@DONE: lea     esp,[ebp-3*4]
        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        ret

drawimager2clip ENDP



;-------------------------------------------------
;core image drawing routine
;no clipping
;y flipping
drawr2  PROC
        ARG @@SRC,@@DEST,@@X,@@Y,@@O

        ;negate diskip if yflip
        mov     eax,@@O
        mov     ebp,_PITCH
        test    eax,1b    ;yflip
        jz      @@NOYFLIP
        mov     eax,[esi.yw]  ;ywidth
        dec eax
        mul     ebp
        add     edi,eax    ;point to last scanline
        neg     ebp
@@NOYFLIP:

        ;get image dimensions
        mov     ebx,[esi.xw]   ;xwidth
        mov     edx,[esi.yw] ;ywidth


        sub     ebp,ebx    ;ebp=diskip

        xor     eax,eax
        xor     ecx,ecx

        ;point to data
        lea     esi,[esi+edx*4+16]

@@YLOOP:
        push    ebx  ;save xwidth
@@XLOOP:

        ;transparent
        mov     al,[esi]
        inc     esi
        add     edi,eax
        sub     ebx,eax
        jle     @@DONELINE

        ;opaque
        mov     al,[esi]
        inc     esi
        xor     ecx,ecx
        sub     ebx,eax
;---------------------------------
;copy forward esi->edi, eax bytes ecx=0
                sub     ecx,edi  ;amount to pre-copy
                and     ecx, 3
                sub     eax, ecx
                jle     short @@ENDCOPY
               rep movsb
                mov     ecx, eax ;main copy
                and     eax, 3
                shr     ecx, 2
               rep movsd
@@ENDCOPY:      add     ecx, eax  ;end copy
                xor     eax,eax
               rep movsb
;done copy
;--------------------------------

        test    ebx,ebx
        jg      @@XLOOP

@@DONELINE:
        pop     ebx ;restore xwidth

        add     edi,ebp ;position to next line

        dec     edx
        jnz     @@YLOOP

        ret

@@DONE: pop ebx
        ret
drawr2  ENDP




;core image drawing routine
;no clipping
;y flipping
drawr2_xflip  PROC
        ARG @@SRC,@@DEST,@@X,@@Y,@@O


        ;negate diskip if yflip
        mov     eax,@@O
        mov     ebp,_PITCH
        test    eax,1b    ;yflip
        jz      @@NOYFLIP
        mov     eax,[esi.yw]  ;ywidth
        dec eax
        mul     ebp
        add     edi,eax    ;point to last scanline
        neg     ebp
@@NOYFLIP:

        ;get image dimensions
        mov     ebx,[esi.xw]   ;xwidth
        mov     edx,[esi.yw] ;ywidth
        ;point to data
        lea     esi,[esi+edx*4+16]

        add     ebp,ebx    ;ebp=diskip (add cause we're flipped)

        add     edi,ebx    ;point to end of scanline to copy leftwards

        xor     eax,eax
        xor     ecx,ecx

@@YLOOP:
        push    ebx  ;save xwidth
@@XLOOP:

        ;transparent
        mov     al,[esi]
        inc     esi
        sub     edi,eax
        sub     ebx,eax
        jle     @@DONELINE

        ;opaque
        mov     al,[esi]
        inc     esi
        sub     ebx,eax

        ;copy backward esi->edi, ecx bytes
;@@COPY: dec     edi
;        mov     al,[esi]
;        inc     esi
;        mov     [edi],al
;        dec     ecx
;        jnz      @@COPY


;;;backwards memory copy! esi->~edi ecx=count
 ;;precopy
                mov     ecx,edi
                and     ecx, 3
                jz      @@NOC1
                sub     eax, ecx
                jle     short @@LEndBytes
                push  eax
@@C1:           dec   edi
                mov   al,[esi]       ; rep movsb
                inc   esi
                mov   [edi],al
                dec   ecx
                jg   @@C1
                pop   eax
 ;;copy
@@NOC1:         mov     ecx, eax
                and     eax, 3
                shr     ecx, 2

                jz @@LEndBytes ;rep movsd
                push eax
@@COPY:         sub  edi,4
                mov  eax,[esi]
                add  esi,4
                bswap eax
                dec  ecx
                mov  [edi],eax
                jnz  @@COPY
                pop eax
 ;;postcopy
@@LEndBytes:    add     ecx, eax

                jle @@DONEC3 ;  rep movsb
@@C3:           dec   edi
                mov   al,[esi]       ; rep movsb
                inc   esi
                mov   [edi],al
                dec   ecx
                jg  @@C3
@@DONEC3:
;;------------------------------------
               xor eax,eax



        test    ebx,ebx
        jg      @@XLOOP

@@DONELINE:
        pop     ebx ;restore xwidth

        add     edi,ebp ;position to next line

        dec     edx
        jnz     @@YLOOP

        ret

@@DONE: pop ebx
ret
drawr2_xflip  ENDP





;core image drawing routine
;xy clipping
;y flipping
drawr2_xyclip  PROC

        ARG     @@DISKIP,@@O,@@XLEN,@@XSTART,@@YLEN,@@YSTART
        push    ebp
        mov     ebp,esp

        ;negate diskip if yflip
        mov     eax,@@O
        test    eax,1b    ;yflip
        jz      @@NOYFLIP
        mov     eax,[esi.yw]
        sub     eax,@@YLEN
        sub     eax,@@YSTART
        mov     @@YSTART,eax

        mov     eax,[@@YLEN]  ;ywidth
        dec     eax
        mul     [@@DISKIP]
        add     edi,eax    ;point to last scanline
        neg     [@@DISKIP]
@@NOYFLIP:

        xor     eax,eax
        xor     ecx,ecx

        push    esi ;save @@SRC
@@YLOOP:
        ;point to source scanline
        pop     esi ;esi=@@SRC
        mov     ebx,[@@YSTART]
        push    esi
        add     esi,[esi+ebx*4+16]
        inc     [@@YSTART]

        ;get image dimensions
        mov     ebx,@@XLEN   ;number of pixels to draw horizontally
        mov     edx,@@XSTART ;number of pixels to skip per scanline before drawing
        push    edi          ;save dest


@@XLOOP:
;------------------------------------------------
        ;skip through XSTART
        test    edx,edx
        jle     @@PHASE2
@@PHASE1:
        ;transparent
@@T1:   mov     al,[esi]
        inc     esi
        sub     edx,eax
        jle     @@T1DONE
        
        ;opaque
@@O1:   mov     al,[esi]
        inc     esi
        add     esi,eax
        sub     edx,eax
        jg      @@PHASE1
        
        ;opaque phase 1 completed
        xor     ecx,ecx
        add     esi,edx  ;scoot back left
        sub     ecx,edx  ;eax=-edx (amt left to copy)
        jmp     @@O2
        
@@T1DONE: ;transparent phase 1 completed
        sub     edi,edx
        add     ebx,edx
        jle     @@DONELINE
        mov     cl,[esi] ;resume at opaque phase2
        inc     esi
        jmp     @@O2

;------------------------------------------------
        ;draw through XLEN
@@PHASE2:
        ;transparent
        mov     al,[esi]
        inc     esi
        add     edi,eax
        sub     ebx,eax
        jle     @@DONELINE

        ;opaque
        mov     cl,[esi]
        inc     esi
@@O2:   sub     ebx,ecx    ;if ebx<0 ecx+=ebx
        jg      @@NORIGHTCLIP
        add     ecx,ebx
@@NORIGHTCLIP:
   ;     rep     movsb
;---------------------------------
;copy forward esi->edi, eax bytes ecx=0
                                         mov     eax,ecx
                sub     ecx,edi  ;amount to pre-copy
                sub     ecx,eax
                and     ecx, 3
                sub     eax, ecx
                jle     short @@ENDCOPY
               rep movsb
                mov     ecx, eax ;main copy
                and     eax, 3
                shr     ecx, 2
               rep movsd
@@ENDCOPY:      add     ecx, eax  ;end copy
                xor     eax,eax
               rep movsb
;done copy
;--------------------------------


        test    ebx,ebx
        jg      @@PHASE2
        
@@DONELINE:
        pop     edi        ;restore dest
        add     edi,@@DISKIP ;position to next line
        
        dec     [@@YLEN]
        jnz     @@YLOOP

@@DONE: pop     esi
        pop     ebp
        ret     6*4   ;return and pop
drawr2_xyclip  ENDP



;core image drawing routine
;xy clipping
;xy flipping
drawr2_xyclip_xflip  PROC

        ARG     @@DISKIP,@@O,@@XLEN,@@XSTART,@@YLEN,@@YSTART
        push    ebp
        mov     ebp,esp

        mov     eax,[esi.xw]    ;ax=xlength
        sub     eax,@@XLEN
        sub     eax,@@XSTART
        mov     @@XSTART,eax

        ;negate diskip if yflip
        mov     eax,@@O
        test    eax,1b    ;yflip
        jz      @@NOYFLIP
        mov     eax,[esi.yw]
        sub     eax,@@YLEN
        sub     eax,@@YSTART
        mov     @@YSTART,eax
        
        mov     eax,[@@YLEN]  ;ywidth
        dec     eax
        mul     [@@DISKIP]
        add     edi,eax    ;point to last scanline
        neg     [@@DISKIP]   
@@NOYFLIP:        
        
        xor     eax,eax
        xor     ecx,ecx
        
        add     edi,@@XLEN    ;point to end of scanline to copy leftwards

        
        push    esi ;save @@SRC
@@YLOOP:
        ;point to source scanline
        pop     esi ;esi=@@SRC
        mov     ebx,[@@YSTART]
        push    esi 
        add     esi,[esi+ebx*4+16] 
        inc     [@@YSTART]
        
        ;get image dimensions        
        mov     ebx,@@XLEN   ;number of pixels to draw horizontally
        mov     edx,@@XSTART ;number of pixels to skip per scanline before drawing
        push    edi          ;save dest
        
        
@@XLOOP:
;------------------------------------------------
        ;skip through XSTART
        test    edx,edx
        jle     @@PHASE2
@@PHASE1:
        ;transparent
@@T1:   mov     al,[esi]
        inc     esi
        sub     edx,eax
        jle     @@T1DONE
        
        ;opaque
@@O1:   mov     al,[esi]
        inc     esi
        add     esi,eax
        sub     edx,eax
        jg      @@PHASE1
        
        ;opaque phase 1 completed
        xor     ecx,ecx
        add     esi,edx  ;scoot back left
        sub     ecx,edx  ;eax=-edx (amt left to copy)
        jmp     @@O2
        
@@T1DONE: ;transparent phase 1 completed
        add     edi,edx
        add     ebx,edx
        jle     @@DONELINE
        mov     cl,[esi] ;resume at opaque phase2
        inc     esi
        jmp     @@O2
     
;------------------------------------------------        
        ;draw through XLEN
@@PHASE2:        
        ;transparent
        mov     al,[esi]
        inc     esi
        sub     edi,eax
        sub     ebx,eax
        jle     @@DONELINE

        ;opaque
        mov     cl,[esi]
        inc     esi
@@O2:   sub     ebx,ecx    ;if ebx<0 ecx+=ebx
        jg      @@NORIGHTCLIP
        add     ecx,ebx
@@NORIGHTCLIP:

        test    ecx,ecx
        jle      @@NOCOPY
        ;copy backward esi->edi, ecx bytes
@@COPY: dec     edi
        mov     al,[esi]
        inc     esi
        mov     [edi],al
        dec     ecx
        jnz      @@COPY
@@NOCOPY:
        test    ebx,ebx
        jg      @@PHASE2

@@DONELINE:
        pop     edi        ;restore dest
        add     edi,@@DISKIP ;position to next line

        dec     [@@YLEN]
        jnz     @@YLOOP

@@DONE: pop     esi
        pop     ebp
        ret     6*4   ;return and pop
drawr2_xyclip_xflip  ENDP





_drawhline PROC
 ARG @@DEST,@@COLOR,@@X,@@Y,@@X2
                        push ebp
         mov    ebp,esp
         push edi

         ;vertical clipping
         mov    eax,@@Y
         test   eax,eax
         jl             @@DONE
         cmp    eax,_SCREENY
         jge    @@DONE

         mov    eax,@@X
         cmp    eax,_SCREENX
         jge    @@DONE
         test   eax,eax
         jge    @@NOLEFTCLIP
         xor    eax,eax
@@NOLEFTCLIP:

         mov    ecx,@@X2
         test  ecx,ecx
         jl             @@DONE
         cmp    ecx,_SCREENX
         jl             @@NORIGHTCLIP
         mov    ecx,_SCREENX
         dec    ecx
@@NORIGHTCLIP:

                        ;inc   ecx
         sub    ecx,eax ;;get length
         jle    @@DONE

                        ;get pointer to dest
         mov    edi,@@DEST
         add    edi,eax

         mov    eax,@@Y
                        mul     _PITCH
         add    edi,eax


         ;set up storing word
         mov    al,byte ptr @@COLOR
         mov    ah,al
         mov    edx,eax
         shl    eax,16
         mov    ax,dx

;        rep stosb
;---------------------------------
;copy forward esi->edi, eax bytes ecx=0
                                    mov     edx,ecx
                sub     ecx,edi  ;amount to pre-copy
                sub     ecx,edx
                and     ecx, 3
                sub     edx, ecx
                jle     short @@ENDCOPY
               rep stosb
                mov     ecx, edx ;main copy
                and     edx, 3
                shr     ecx, 2
               rep stosd
@@ENDCOPY:      add     ecx, edx  ;end copy
               rep stosb
;done store
;--------------------------------


@@DONE:
                        pop   edi
         pop    ebp
         ret
_drawhline ENDP





_drawvline PROC
  ARG @@DEST,@@COLOR,@@X,@@Y,@@Y2
                        push ebp
         mov    ebp,esp
         push edi

         ;horiz clipping
         mov    eax,@@X
         test   eax,eax
         jl             @@DONE
         cmp    eax,_SCREENX
         jge    @@DONE

         mov    eax,@@Y
         cmp    eax,_SCREENY
         jg     @@DONE
         test   eax,eax
         jg             @@NOTOPCLIP
         xor    eax,eax
@@NOTOPCLIP:

         mov    ecx,@@Y2
         test  ecx,ecx
         jl             @@DONE
         cmp    ecx,_SCREENY
         jl             @@NOBOTTOMCLIP
         mov    ecx,_SCREENY
         dec    ecx
@@NOBOTTOMCLIP:

                        ;inc   ecx
         sub    ecx,eax ;;get length
         jle   @@DONE

                        ;get pointer to dest
         mov    edi,@@DEST
                        mul     _PITCH
         add    edi,eax
         add    edi,@@X

         ;set up storing word
         mov    al,byte ptr @@COLOR
         mov    edx,[_PITCH]

@@LP:           mov     [edi],al
                        add     edi,edx
         dec    ecx
         jnz    @@LP

@@DONE:
         pop   edi
         pop    ebp
         ret
_drawvline ENDP







_drawrect PROC
 ARG @@B:DWORD,@@COLOR:DWORD,@@X:DWORD,@@Y:DWORD,@@XW:DWORD,@@YW:DWORD

          push ebp
          mov ebp,esp
          push ebx
          push edi

          mov eax,_SCREENX
          cmp [@@X],eax
          jge @@DONE

          mov eax,_SCREENY
          cmp [@@Y],eax
          jge @@DONE


          mov eax,_SCREENX
          mov ebx,@@X
          add ebx,@@XW
          cmp ebx,eax
          jl @@NOXO
          sub eax,@@X
          mov @@XW,eax
@@NOXO:

          mov eax,_SCREENY
          mov ebx,@@Y
          add ebx,@@YW
          cmp ebx,eax
          jl @@NOYO
          sub eax,@@Y
          mov @@YW,eax
@@NOYO:


          cmp @@X,0
          jge  @@NOXU
          mov eax,@@X
          add [@@XW],eax
          mov @@X,0
@@NOXU:

          cmp @@Y,0
          jge  @@NOYU
          mov eax,@@Y
          add [@@YW],eax
          mov @@Y,0
@@NOYU:

          cmp [@@XW],0
          jle @@DONE
          cmp [@@YW],0
          jle @@DONE

          mov  eax,@@Y
          mul  _PITCH
          add  eax,@@X
          mov  edi,@@B
          add  edi,eax

          mov  ebx,@@COLOR
          mov  al,bl
          shl  eax,8
          mov  al,bl
          shl  eax,8
          mov  al,bl
          shl  eax,8
          mov  al,bl

          cld
@@YLOOP:
        mov ecx, @@XW


;---------------------------------
;copy forward esi->edi, eax bytes ecx=0
                                    mov     edx,ecx
                sub     ecx,edi  ;amount to pre-copy
                sub     ecx,edx
                and     ecx, 3
                sub     edx, ecx
                jle     short @@ENDCOPY
               rep stosb
                mov     ecx, edx ;main copy
                and     edx, 3
                shr     ecx, 2
               rep stosd
@@ENDCOPY:      add     ecx, edx  ;end copy
               rep stosb
;done store
;--------------------------------

        add edi,_PITCH
        sub edi,@@XW

        dec @@YW
        jg @@YLOOP

@@DONE:
        pop edi
        pop ebx
        pop ebp
        ret

_drawrect ENDP



;draw rectangle with color mapping
_drawrectmap PROC
 ARG @@B:DWORD,@@MAP:DWORD,@@X:DWORD,@@Y:DWORD,@@XW:DWORD,@@YW:DWORD

          push ebp
          mov ebp,esp
          push ebx
          push esi
          push edi

          mov eax,_SCREENX
          cmp [@@X],eax
          jge @@DONE

          mov eax,_SCREENY
          cmp [@@Y],eax
          jge @@DONE

          mov eax,_SCREENX
          mov ebx,@@X
          add ebx,@@XW
          cmp ebx,eax
          jl @@NOXO
          sub eax,@@X
          mov @@XW,eax
@@NOXO:

          mov eax,_SCREENY
          mov ebx,@@Y
          add ebx,@@YW
          cmp ebx,eax
          jl @@NOYO
          sub eax,@@Y
          mov @@YW,eax
@@NOYO:


          cmp @@X,0
          jge  @@NOXU
          mov eax,@@X
          add [@@XW],eax
          mov @@X,0
@@NOXU:

          cmp @@Y,0
          jge  @@NOYU
          mov eax,@@Y
          add [@@YW],eax
          mov @@Y,0
@@NOYU:

          cmp [@@XW],0
          jle @@DONE
          cmp [@@YW],0
          jle @@DONE

          ;point edi to region to draw to
          mov  eax,@@Y
          mul  _PITCH
          add  eax,@@X
          mov  edi,@@B
          add  edi,eax


          mov  esi,@@MAP
          xor  edx,edx
          xor  ecx,ecx
@@YLOOP:
          ;dword color mapping
          push edi
          push ebp

;precopy
                         mov  ecx,@@XW
          and  ecx,11b
          jz   @@NOPRECOPY
@@PRECOPY:mov   dl,[edi]      ;1U
                           ;2stall
                         mov  al,[esi+edx]  ;3U
          mov  [edi],al      ;4U
                         inc    edi           ;4V
                    dec ecx           ;5U
          jnz   @@PRECOPY       ;5V
@@NOPRECOPY:

;dword copy
          mov  eax,[edi] ;load first word
          mov  ebp,@@XW
          shr  ebp,2
@@COPY:
          ror  eax,16        ;1 U unpairable

          mov  dl,al         ;2 U
          mov  cl,ah         ;2 V

          shr  eax,16        ;3 U
          add  edi,4         ;3 V

          mov  bl,[esi+edx]  ;4 U
          mov  dl,al         ;4 V

          mov  bh,[esi+ecx]  ;5 U
          mov  cl,ah         ;5 V

          shl  ebx,16        ;6 U

          mov  bl,[esi+edx]  ;7 U
          mov   eax,[edi]     ;7 V

          mov  bh,[esi+ecx]  ;8 U
          dec ebp           ;8 V

          mov  [edi-4],ebx   ;9 U
          jnz   @@COPY        ;9 V

          pop  ebp
          pop  edi
          add   edi,_PITCH

          dec @@YW
          jg @@YLOOP

@@DONE:
          pop edi
          pop esi
          pop ebx
          pop ebp
          ret
_drawrectmap ENDP











_drawscr PROC
    ARG @@SRC,@@DEST,@@XW,@@YW,@@PITCH

                push    ebp
         mov    ebp,esp

         push   esi
         push   edi

         mov    esi,@@SRC
         mov    edi,@@DEST

         mov    eax,@@XW
@@LP:
                        xor     ecx,ecx

;---------------------------------
;copy forward esi->edi, eax bytes ecx=0
                sub     ecx,edi  ;amount to pre-copy
                and     ecx, 3
                sub     eax, ecx
                jle     short @@ENDCOPY
               rep movsb
                mov     ecx, eax ;main copy
                and     eax, 3
                shr     ecx, 2
               rep movsd
@@ENDCOPY:      add     ecx, eax  ;end copy
                xor     eax,eax
               rep movsb
;done copy
;--------------------------------

         mov    eax,@@XW

         add    esi,@@PITCH
         add    edi,_PITCH
         sub   esi,eax
         sub   edi,eax
                dec     @@YW
         jnz    @@LP

@@DONE:
         pop    edi
         pop    esi
         pop    ebp
         ret
_drawscr ENDP
















comment *
;single byte mapping
          cld
          xor  eax,eax
          mov  ebx,@@MAP
          mov  ecx,@@XW
          xor  edx,edx

@@YLOOP:
@@XLOOP:  mov   al,[edi]      ;1U
                           ;2stall
                         mov  dl,[ebx+eax]  ;3U
          mov  [edi],dl      ;4U
                         inc    edi           ;4V
                    dec ecx           ;5U
          jnz   @@XLOOP       ;5V

          mov  ecx,@@XW
          add   edi,_PITCH
          sub   edi,ecx

          dec @@YW
          jg @@YLOOP

*



        END






       
        
        
        

        
        
        
        
        
        
        
        
                

                
                
                
                
                

