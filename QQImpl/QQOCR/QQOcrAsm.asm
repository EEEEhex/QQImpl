.CODE

; 以下为MMMojo的实现
OnReadPushTransfer PROC
	mov rax, [r8]    
	mov r9, r8                        
	mov r8, rdx                       
	mov edx, ecx                      
	mov rcx, r9                       
	jmp qword ptr [rax + 8h]
OnReadPushTransfer ENDP

OnReserved2Transfer PROC
	mov rax, [r8]
	mov r9, r8                
	mov r8, rdx               
	mov edx, ecx              
	mov rcx, r9               
	jmp qword ptr [rax + 10h]
OnReserved2Transfer ENDP

OnReserved3Transfer PROC
	mov rax, [r8]
	mov r9, r8
	mov r8, rdx
	mov edx, ecx
	mov rcx, r9
	jmp qword ptr [rax + 18h]
OnReserved3Transfer ENDP

OnRemoteConnectTransfer PROC
	mov rax, [rdx]
	mov r8, rdx
	movzx edx, cl
	mov rcx, r8
	jmp qword ptr [rax + 20h]
OnRemoteConnectTransfer ENDP

OnRemoteDisconnectTransfer PROC
	mov rax, [rcx]
	jmp qword ptr [rax + 28h]
OnRemoteDisconnectTransfer ENDP

OnRemoteProcessLaunchedTransfer PROC
	mov rax, [rcx]
	jmp qword ptr [rax + 30h]
OnRemoteProcessLaunchedTransfer ENDP

OnRemoteProcessLaunchFailedTransfer PROC
	mov rax, [rdx]
	mov r8, rdx
	mov edx, ecx
	mov rcx, r8
	jmp qword ptr [rax + 38h]
OnRemoteProcessLaunchFailedTransfer ENDP

OnRemoteMojoErrorTransfer PROC
	mov rax, [r8]
	mov r9, r8
	mov r8d, edx
	mov rdx, rcx
	mov rcx, r9
	jmp qword ptr [rax + 40h]
OnRemoteMojoErrorTransfer ENDP

END