                .486
                .MODEL  flat
                LOCALS

                .DATA
                EXTRN _PITCH:DWORD,_SCREENX:DWORD,_SCREENY:DWORD

XSTART DD 0
YSTART DD 0

                .CODE

					 PUBLIC _draw_sprite_asm


_draw_sprite_asm  PROC  NEAR
                ARG @@B,@@DEST,@@X,@@Y,@@O,@@PAL


               push 	ebp
               mov	ebp,esp
               push	ebx

         ;;clipping checks
        xor     eax,eax
        mov     ecx,@@X
        mov     edx,@@Y

        add     ecx,8 ;right extent of image
        add     edx,8 ;bottom extent of image

        cmp     [@@X],eax    ;left clip
        jl      draw_sprite_asm_clip
        cmp     [_SCREENX],ecx ;right clip
        jl      draw_sprite_asm_clip
        cmp     [@@Y],eax      ;top clip
        jl      draw_sprite_asm_clip
        cmp     [_SCREENY],edx ;bottom clip
        jl      draw_sprite_asm_clip


;;--------------------------
;;normal drawing
					push	esi
               push	edi

               mov	eax,[@@Y]
					mul	[_PITCH]
               add	eax,[@@X]
               mov	edi,[@@DEST]
					mov	esi,[@@B] ;get pointer to sprite
               add	edi,eax ;di->dest si->sprite
               mov	edx,8 ;;number of lines

               mov   ebx,[_PITCH]
               sub   ebx,8

               mov cl,[byte ptr @@PAL]
               shl cl,2    ;get palette
               or  cl,224+16   ;or in colorbase

               ;call routine based on orientation
               mov   al,[byte ptr @@O]
               test  al,10b ;yflip
               jnz	@@YFLIP
               test  al,1b  ;xflip
               jnz   draw_flipx_noclip
               jmp   draw_noflip_noclip
@@YFLIP:       test  al,1b  ;xflip
               jnz   draw_flipxy_noclip
               jmp   draw_flipy_noclip

DONESPRITE:    pop	edi
               pop 	esi
               pop	ebx
               pop	ebp
               ret

_draw_sprite_asm  ENDP

;-------------------------
;actual drawing functions

;esi->sprite
;edi->dest
;ebx=edi_skip
;edx=ylen (8)

;no flipping/no clipping
draw_noflip_noclip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase


@@YLP:         mov   ecx,8
@@XLP: 		   mov	al,[esi]
					inc	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

	            add	edi,ebp
               dec  edx
               jg   @@YLP

               jmp  DONESPRITE
draw_noflip_noclip ENDP



;flipping y/no clipping
draw_flipy_noclip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

					add   esi,8*8-8 ;point to beginning of last scanlin

@@YLP:         mov   ecx,8
@@XLP: 		   mov	al,[esi]
					inc	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

	            add	edi,ebp
               sub   esi,16
               dec   edx
               jg   @@YLP

               jmp  DONESPRITE
draw_flipy_noclip ENDP

;flip x/no clipping
draw_flipx_noclip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

               add   esi,8-1

@@YLP:         mov   ecx,8
@@XLP: 		   mov	al,[esi]
					dec	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

	            add	edi,ebp
               add   esi,16
               dec   edx
               jg   @@YLP

               jmp  DONESPRITE
draw_flipx_noclip ENDP

;flip xy/no clipping
draw_flipxy_noclip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

               add esi,8*8-1

@@YLP:         mov   ecx,8
@@XLP: 		   mov	al,[esi]
					dec	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

	            add	edi,ebp
               dec   edx
               jg   @@YLP
               jmp  DONESPRITE
draw_flipxy_noclip ENDP



;;-----------------------------------------------------------------
;;-----------------------------------------------------------------
;;-----------------------------------------------------------------
;;-----------------------------------------------------------------
;;clipping



draw_sprite_asm_clip  PROC
        ARG @@B,@@DEST,@@X,@@Y,@@O,@@PAL

        push	esi
        push	edi
        mov	   esi,@@B
        mov	   edi,@@DEST
;        jmp  @@DONE
;mov @@O,0

;top clip
        xor     eax,eax
        xor     ebx,ebx
        cmp     [@@Y],eax
        jg      @@NOTOP
        cmp     edx,eax ;top edge is <0
        jle     DONESPRITE
        sub     eax,@@Y ;ystart=0-Y
        add     @@Y,eax ;Y=0
        sub		 ebx,eax     ;;save ystart
@@NOTOP:
        mov     [YSTART],eax

;bottom clip
        mov     eax,8 ;YTOTAL
        sub     edx,[_SCREENY]
        jle     @@NOBOTTOM
        sub     eax,edx ;ylen=SCREENY-Y
        jle     DONESPRITE
@@NOBOTTOM:
        lea		edx,[eax+ebx] ;YLEN=YTOTAL-YSTART

;left clip
        xor     eax,eax
        xor 	 ebx,ebx
        cmp     [@@X],eax
        jg      @@NOLEFT
        cmp     ecx,eax ;right edge is <0
        jle     DONESPRITE
        sub     eax,@@X   ;xstart=0-X
        add     [@@X],eax ;X=0
        sub 	 ebx,eax  ;save -xstart
@@NOLEFT:
        mov     [XSTART],eax

;right clip
        mov     eax,8
        sub     ecx,[_SCREENX]
        jle     @@NORIGHT
        sub     eax,ecx
        jle     DONESPRITE
@@NORIGHT:
		  add     ebx,eax ;store XLEN=XTOTAL + (-XSTART)


        ;point to memory for dest
        push   edx
        mov     eax,_PITCH
        mul     @@Y
        add     eax,@@X
        add     edi,eax
        pop		 edx

        mov cl,[byte ptr @@PAL]
        shl cl,2    ;get palette
        or  cl,224+16   ;or in colorbase

        ;call routine based on orientation
        mov   eax,[@@O]
        test  al,10b ;yflip
        jnz	@@YFLIP
        test  al,1b  ;xflip
        jnz   draw_flipx_clip
        jmp   draw_noflip_clip
@@YFLIP:test  al,1b  ;xflip
        jnz   draw_flipxy_clip
        jmp   draw_flipy_clip

draw_sprite_asm_clip  ENDP

;-------------------------------------------

;clipped and not flipped
draw_noflip_clip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

              add esi,[XSTART]
              mov eax,[YSTART]
              shl eax,3
              add esi,eax

@@YLP:         mov   ecx,ebp ;xlen
@@XLP: 		   mov	al,[esi]
					inc	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

               sub   esi,ebp
               sub   edi,ebp

               add   esi,8
               add   edi,_PITCH

               dec   edx
               jg   @@YLP
               jmp DONESPRITE
draw_noflip_clip ENDP


;clipped and flipped
draw_flipy_clip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

              add esi,[XSTART]
              mov eax,[YSTART]
              xor eax,111b
              shl eax,3
              add esi,eax

@@YLP:         mov   ecx,ebp ;xlen
@@XLP: 		   mov	al,[esi]
					inc	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

               sub   esi,ebp
               sub   edi,ebp

               sub   esi,8
               add   edi,_PITCH

               dec   edx
               jg   @@YLP
               jmp DONESPRITE
draw_flipy_clip ENDP


;clipped and flipped
draw_flipx_clip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

              mov eax,[XSTART]
              xor eax,111b
              add esi,eax
              mov eax,[YSTART]
              shl eax,3
              add esi,eax

@@YLP:         mov   ecx,ebp ;xlen
@@XLP: 		   mov	al,[esi]
					dec	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

               add   esi,ebp
               sub   edi,ebp

               add   esi,8
               add   edi,_PITCH

               dec   edx
               jg   @@YLP
               jmp DONESPRITE
draw_flipx_clip ENDP


;clipped and flipped
draw_flipxy_clip PROC
            mov ebp,ebx ;diskip
            mov bl,cl ;colorbase

              mov eax,[XSTART]
              xor eax,111b
              add esi,eax
              mov eax,[YSTART]
              xor eax,111b
              shl eax,3
              add esi,eax

@@YLP:         mov   ecx,ebp ;xlen
@@XLP: 		   mov	al,[esi]
					dec	esi
               test  al,al
               jz    @@NODRAW
               or    al,bl
               mov   [edi],al
@@NODRAW:      inc   edi
               dec   ecx
               jg    @@XLP

               add   esi,ebp
               sub   edi,ebp

               sub   esi,8
               add   edi,_PITCH

               dec   edx
               jg   @@YLP
               jmp DONESPRITE
draw_flipxy_clip ENDP







					END












