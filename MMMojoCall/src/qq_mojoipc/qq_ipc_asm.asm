.CODE

; 内部会用到movaps xmmword指令, 此指令必须要求rsp是16位栈对齐的, 因此需要构造rsp的最后一位为0

InitLogAsm PROC
	call r9							; 第4个参数是函数地址
	ret
InitLogAsm ENDP

InitParentIpcAsm PROC
	sub rsp, 48h
	call rdx						; 第2个参数是函数地址
	add rsp, 48h
	ret
InitParentIpcAsm ENDP

LaunchChildProcessAsm PROC
	sub rsp, 48h
	mov rax, [rsp + 70h]			; [rsp + 48h + 28h]原第5个参数
	mov [rsp + 20h], rax
	mov rax, [rsp + 78h]			; [rsp + 48h + 30h]原第6个参数
	mov [rsp + 28h], rax
	mov rax, [rsp + 80h]			; [rsp + 48h + 38h]原第7个参数 也就是func_addr
	call rax
	add rsp, 48h
	ret
LaunchChildProcessAsm ENDP

ConnectedToChildProcessAsm PROC
	sub rsp, 48h
	call r8							; 第3个参数是函数地址
	add rsp, 48h
	ret
ConnectedToChildProcessAsm ENDP

SendIpcMessageAsm PROC
	sub rsp, 48h
	mov rax, [rsp + 70h]			; [rsp + 48h + 28h]原第5个参数
	mov [rsp + 20h], rax
	mov rax, [rsp + 78h]			; [rsp + 48h + 30h]原第6个参数
	mov [rsp + 28h], rax
	mov rax, [rsp + 80h]			; [rsp + 48h + 38h]原第7个参数 也就是func_addr
	call rax
	add rsp, 48h
	ret
SendIpcMessageAsm ENDP

TerminateChildProcessAsm PROC
	sub rsp, 48h
	mov rax, [rsp + 70h]			; [rsp + 48h + 28h]原第5个参数 也就是func_addr
	call rax
	add rsp, 48h
	ret
TerminateChildProcessAsm ENDP

ReLaunchChildProcessAsm PROC
	sub rsp, 48h
	call r8							; 第3个参数是函数地址
	add rsp, 48h
	ret
ReLaunchChildProcessAsm ENDP

InitChildIpcAsm PROC
	sub rsp, 48h
	call rdx						; 第2个参数是函数地址
	add rsp, 48h
	ret
InitChildIpcAsm ENDP

SetChildReceiveCallbackAndCOMPtrAsm PROC
	sub rsp, 48h
	call r9							; 第4个参数是函数地址
	add rsp, 48h
	ret
SetChildReceiveCallbackAndCOMPtrAsm ENDP

ChildSendIpcMessageAsm PROC
	sub rsp, 48h
	mov rax, [rsp + 70h]			; [rsp + 48h + 28h]原第5个参数
	mov [rsp + 20h], rax
	mov rax, [rsp + 78h]			; [rsp + 48h + 30h]原第6个参数 也就是func_addr
	call rax
	add rsp, 48h
	ret
ChildSendIpcMessageAsm ENDP

END