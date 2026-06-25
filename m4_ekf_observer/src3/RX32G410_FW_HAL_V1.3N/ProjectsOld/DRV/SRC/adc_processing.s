; adc_processing.s
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

; 外部变量引用
    IMPORT TxA_ADC_AdcDmaBuff
	IMPORT	APP_ADC_Dntr

;    IMPORT block_idx

; 函数导出
    EXPORT 	APP_ADC_AvgTxA20us
	EXPORT	APP_ADC_IRQ_PPGstepDecTxACallBack_ASM
	EXPORT	API_POWER_PanCheckPluseAsm	
	
TxA_ADC_AdcDMA_BUFF_NUM		equ 	32		
HTRIM_TEST1_REG				equ		0x40016900		
HTRIM_TEST1_ICR				equ		0x88			;icr偏移地址	
HTRIM_TEST1_DIER			equ		0x8c			;ier
; 函数实现
APP_ADC_AvgTxA20us PROC
    ; 保存寄存器 (根据AAPCS)
    PUSH    {R4-R11, LR}
    
    ; 加载block_idx地址
;    LDR     R0, =block_idx
;    LDRB    R2, [R0]           ; R2 = block_idx
    
    ; 加载结构体基地址
    LDR     R1, =TxA_ADC_AdcDmaBuff+4		;VOLTAGE指针

;R1 VOLTAGE   R4 T12A   R5  T34A    
    ; 计算数据偏移: block_idx * 32 (8 samples * 4 bytes)
;    LSL     R3, R2, #5         ; R3 = block_idx * 32

    ; 设置各通道指针
    ADD     R4, R1, #8         ; T12A指针
    ADD     R5, R4, #128       ; T34A指针 (128 = 32 samples * 4 bytes)
;    ADD     R6, R4, #256       ; T3A指针
;    ADD     R7, R4, #384       ; T4A指针
    
    ; 清零累加器
    MOV     R8, #0             ; sum1
    MOV     R10, #0             ; sum2
;    MOV     R10, #0            ; sum3
;    MOV     R11, #0            ; sum4
    
    ; === 分组处理样本 ===
    ; 第1组 (索引0-1)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12
    
;    LDMIA   R6!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12
;    
;    LDMIA   R7!, {R0, R12}
;    ADD     R11, R11, R0
;    ADD     R11, R11, R12
    
    ; 第2组 (索引2-3)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12
    
;    LDMIA   R6!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12
;    
;    LDMIA   R7!, {R0, R12}
;    ADD     R11, R11, R0
;    ADD     R11, R11, R12
    
    ; 第3组 (索引4-5)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12
    
;    LDMIA   R6!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12
;    
;    LDMIA   R7!, {R0, R12}
;    ADD     R11, R11, R0
;    ADD     R11, R11, R12
    
    ; 第4组 (索引6-7)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12
    
;    LDMIA   R6, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12
;    
;    LDMIA   R7, {R0, R12}
;    ADD     R11, R11, R0
;    ADD     R11, R11, R12
   
    ; 第5组 (索引8-9)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12

    ; 第6组 (索引10-11)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12

    ; 第7组 (索引12-13)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12

    ; 第8组 (索引14-15)
    LDMIA   R4!, {R0, R12}
    ADD     R8, R8, R0
    ADD     R8, R8, R12
    
    LDMIA   R5!, {R0, R12}
    ADD     R10, R10, R0
    ADD     R10, R10, R12

;    ; 第9组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12

;    ; 第10组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12	

;    ; 第11组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12

;    ; 第12组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12

;    ; 第13组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12

;    ; 第14组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12	
;	
;	
;    ; 第15组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12

;    ; 第16组 (索引0-1)
;    LDMIA   R4!, {R0, R12}
;    ADD     R8, R8, R0
;    ADD     R8, R8, R12
;    
;    LDMIA   R5!, {R0, R12}
;    ADD     R10, R10, R0
;    ADD     R10, R10, R12	
	
	
    ; === 存储结果 ===

;    ADD     R0, R0, R2, LSL #2 ; R0 = &sumT1A[block_idx]
    
	
	
	
    MOVW	R0,		#0xffff

	UBFX	R9,	    R8,#16,#16
	AND		R8,		R0
	AND		R9,	    R0


	LSR		R11,	    R10,#16
	AND		R10,		R0
	AND		R11,		R0	
;R8 T1ASUM  R9 T2ASUM  R10 T3ASUM R11 T4ASUM
;R8*voltage

	LDMIA	R1, {R6, R7}					;R6 COUNT R7 VOLTAGE R1 COUNT指针
		
	MUL		R8,	R8,	R7		;R8*VOLTAGE
	LSR     R8, R8, #6		;/16 /4 多移两位防止累加越界	
	MUL		R9,	R9,	R7
	LSR     R9, R9, #6 	
	MUL		R10,R10,R7
	LSR     R10, R10, #6 	
	MUL		R11,R11,R7
	LSR     R11, R11, #6 

    ; 计算sum数组地址 (假设sumT1A在偏移512处)
    ADD     R0, R1, #(256+8)       ; R0 sum数组基地址
	MOV		R7,	R0					
	
	LDMIA   R7!, {R4, R5}
	ADD		R8,R4,R8
	ADD		R9,R5,R9
	
	LDMIA   R7!, {R4, R5}
	ADD		R10,R4,R10
	ADD		R11,R5,R11


    STMIA   R0,     {R8-R11}		;R8-R11保存到SUM R0

	ADD		R6,R6,#1
	STR		R6,[R1]
	
;    STR     R8, [R0]           ; sumT1A[block_idx]
;    ADD     R0, R0, #128        ; 移动到sumT2A (4个uint32_t = 16字节)
;    STR     R9, [R0]           ; sumT2A[block_idx]
;    ADD     R0, R0, #128        ; 移动到sumT3A
;    STR     R10, [R0]          ; sumT3A[block_idx]
;    ADD     R0, R0, #128        ; 移动到sumT4A
;    STR     R11, [R0]          ; sumT4A[block_idx]
    
    ; === 更新block_idx ===
;;    LDR     R0, =block_idx
;    LDRB    R1, [R0]
;    ADDS    R1, #1
;    AND     R1, R1, #3         ; MOD 4
;    STRB    R1, [R0]
    
    ; 恢复寄存器并返回
    POP     {R4-R11, PC}
    ENDP



APP_ADC_IRQ_PPGstepDecTxACallBack_ASM PROC
	
    ; 保存寄存器 (根据AAPCS)
    PUSH    {R4-R11, LR}
    MOV     r3, #0            				; i=0 (r3=i)
    LDR     r4, =TxA_ADC_AdcDMA_BUFF_NUM  	; 加载缓冲区大小常量
	LDR		r2,	=APP_ADC_Dntr+4				;dntr[i]
	LDR		r1,	=APP_ADC_Dntr				;num
	LDR     R0, =TxA_ADC_AdcDmaBuff+4+8		;T12指针
_loop_start

    LDR     r5, [r2, r3, LSL #2]  			;加载dntr[i] (r2=dntr数组基址)(r2 + (r3 << 2));
    SUB     r5, r4, r5         ; start = BUFF_NUM - dntr[i]
    AND     r6, r5, #1         ; 检查奇偶性
    CMP     r6, #1
    IT      EQ
    ADDEQ   r5, #1            	; 奇数则start+1 r5 start

    ; 加载4个ADC值到寄存器 (优化内存访问)
    LSL     r5, #1            ; start*2 (uint16_t偏移)
    ADD     r6, r0, r5        ; T12Abuff + start R1 DNTR[1]
    LDRH    r0, [r6]          ; value
    LDRH    r5, [r6, #2]      ; value

    ; 过流检查 (内联判断逻辑)
    CMP     r0, #0x8000       ; 阈值示例0x8000
    IT      HI
    MOVHI   r10, #1       ; 原子写标志位

    CMP     r5, #0x8000
    IT      HI
    MOVHI   r11, #1

    ; 循环控制 (展开5次)
    ADD     r3, #1            ; i++
    CMP     r3, r0            ; r0=num
    BLT     _loop_start

    ; 恢复寄存器并返回
    POP     {R4-R11, PC}
    ENDP
		
;BL.W          API_DMA_GetDmaCndtr (0x08001708)
;STR           r0,[sp,#0x04]


;		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM_TIMICR_CMP1C);

;	LDR			R0,		=HTRIM_TEST1_CR
;	MOVS          r3,#0x01
;	STR           r3,[r0,#HTRIM_TEST1_ICR]	
;;		__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChTest1].num, HRTIM_TIM_IT_CMP1);	
;	
;	LDR				R1,	[r0,#HTRIM_TEST1_IER]
;	ORR           	r1,r1,#0x01
;	STR           	r1,[r0,#HTRIM_TEST1_IER]
	
	

		
		
; 寄存器分配方案
; r0: 循环计数器i
; r1: downCount值
; r2: 位掩码条件寄存器
; r3: precomputed_div基地址
; r4: 当前div值
; r5: 位掩码临时值1 (div>3)
; r6: 位掩码临时值2 (downCount>2)
; r7: 处理逻辑临时寄存器
; r8: Xn (绑定PanPluse.Xn)
; r9: Sn (绑定PanPluse.Sn)

; 初始化阶段

;API_POWER_PanCheckPluseAsm1	PROC


;    PUSH    {R4-R11, LR}        ; 保存寄存器
;    ;===== 初始化部分 =====
;    LDRH    R2, [R0, #16]       ; 加载初始value
;    LSL     R3, R2, #6          ; Sn = value << 6
;    STR     R3, [R0, #4]        ; 存储 Sn
;    STR     R3, [R0, #8]        ; 存储 Sn1
;    STRH    R2, [R0, #0]        ; 存储 Xn
;    STRH    R2, [R0, #2]        ; 存储 Xn1
;    MOV     R5, #0              ; upCount=0
;    MOV     R6, #0              ; downCount=0
;    MOV     R7, #0              ; pluseCount=0
;    MOV     R4, R2              ; Xn = value
;    ADD     R12, R0, #16        ; R12 = value数组基址
;    MOV     R1, #0              ; i=0
;    ;===== 主循环 (展开4次) =====
;loop_start
;    ADD     R8, R12, R1, LSL #1  ; 预计算基址
;    ;--- 第1个数据 ---
;    LDRH    R2, [R8, #0]         ; 加载value[i]
;    SUB     R3, R3, R3, LSR #3   ; Sn -= Sn>>3
;    ADD     R3, R3, R2, LSL #3   ; Sn += newValue<<3
;    SUBS    R11, R2, R4          ; div = newValue - Xn
;    MOV     R4, R3, LSR #6       ; Xn = Sn >> 6
;    CMP     R11, #3
;    IT      GT
;    ADDGT   R5, R5, #1           ; upCount++ 
;    BLE     else_branch1
;back1
;    ;--- 第2个数据 ---
;    LDRH    R2, [R8, #2]         ; 加载value[i+1]
;    SUB     R3, R3, R3, LSR #3
;    ADD     R3, R3, R2, LSL #3
;    SUBS    R11, R2, R4
;    MOV     R4, R3, LSR #6
;    CMP     R11, #3
;    IT      GT
;    ADDGT   R5, #1
;    BLE     else_branch2
;back2
;    ;--- 第3个数据 ---
;    LDRH    R2, [R8, #4]         ; 加载value[i+2]
;    SUB     R3, R3, R3, LSR #3
;    ADD     R3, R3, R2, LSL #3
;    SUBS    R11, R2, R4
;    MOV     R4, R3, LSR #6
;    CMP     R11, #3
;    IT      GT
;    ADDGT   R5, #1
;    BLE     else_branch3
;back3
;    ;--- 第4个数据 ---
;    LDRH    R2, [R8, #6]         ; 加载value[i+3]
;    SUB     R3, R3, R3, LSR #3
;    ADD     R3, R3, R2, LSL #3
;    SUBS    R11, R2, R4
;    MOV     R4, R3, LSR #6
;    CMP     R11, #3
;    IT      GT
;    ADDGT   R5, #1
;    BLE     else_branch4
;back4
;    ADD     R1, R1, #4           ; i += 4
;    CMP     R1, #100
;    BLT     loop_start
;    B       loop_end

;    ;===== ELSE分支处理 =====
;else_branch1
;    ADD     R6, R6, #1           ; downCount++
;    CMP     R6, #2
;    BLE     back1                ; if downCount <= 2, 继续
;    CMP     R5, #2               ; 检查 upCount > 2
;    IT      GT
;    ADDGT   R7, R7, #1           ; pluseCount++ (条件执行)
;    MOV     R5, #0               ; 重置 upCount
;    B       back1

;else_branch2
;    ADD     R6, R6, #1           ; downCount++
;    CMP     R6, #2
;    BLE     back2
;    CMP     R5, #2
;    IT      GT
;    ADDGT   R7, R7, #1
;    MOV     R5, #0
;    B       back2

;else_branch3
;    ADD     R6, R6, #1           ; downCount++
;    CMP     R6, #2
;    BLE     back3
;    CMP     R5, #2
;    IT      GT
;    ADDGT   R7, R7, #1
;    MOV     R5, #0
;    B       back3

;else_branch4
;    ADD     R6, R6, #1           ; downCount++
;    CMP     R6, #2
;    BLE     back4
;    CMP     R5, #2
;    IT      GT
;    ADDGT   R7, R7, #1
;    MOV     R5, #0
;    B       back4

;    ;===== 循环结束 =====
;loop_end
;    ; 存储结果
;    STRB    R5, [R0, #12]        ; upCount
;    STRB    R6, [R0, #13]        ; downCount
;    STRB    R7, [R0, #14]        ; pluseCount

;    POP     {R4-R11, PC}         ; 恢复寄存器并返回
;	ENDP

; Q15格式脉冲检测程序 (1位符号 + 15位小数)
; 输入: R0 = 结构体指针 (含Q15格式数据数组)
; 输出: [R0+12]=upCount, [R0+13]=downCount, [R0+14]=pluseCount
API_POWER_PanCheckPluseAsm PROC
	
    PUSH    {R4-R11, LR}        ; 保存寄存器
    
    ;===== 初始化部分 (Q15) =====
    LDRH    R2, [R0, #16]       ; 加载初始value (Q15)
    LSL     R3, R2, #16         ; 状态变量 = value<<16 (Q15.16格式)
    STR     R3, [R0, #4]        ; 存储Sn (Q15.16)
    STR     R3, [R0, #8]        ; 存储Sn1
    STRH    R2, [R0, #0]        ; 存储Xn (Q15)
    STRH    R2, [R0, #2]        ; 存储Xn1
    MOV     R5, #0              ; upCount=0
    MOV     R6, #0              ; downCount=0
    MOV     R7, #0              ; pluseCount=0
    MOV     R4, R2              ; R4 = 初始滤波值 (Q15)
    ADD     R12, R0, #16        ; R12 = value数组基址
    MOV     R1, #0              ; i=0
    
    ; 阈值计算：原始阈值3 (12位ADC) → Q15格式
    ; 3/1024 * 32768 = 96 (0x0060)
    MOVW    R8, #96             ; Q15阈值 = 96
    
    ;===== 主循环 (展开4次) =====
loop_start
    ADD     R9, R12, R1, LSL #1 ; 计算当前数据地址
    
    ;--- 第1个数据 (Q15处理) ---
    LDRH    R2, [R9, #0]        ; 加载value[i] (Q15)
    
    ; 状态更新: R3 = (7/8)*R3 + (1/8)*(value<<16)
    MOV     R10, R3, LSR #3     ; R10 = R3/8
    SUB     R3, R3, R10         ; R3 = (7/8)*R3
    LSL     R10, R2, #16        ; value<<16 (Q15→Q15.16)
    ADD     R3, R3, R10, LSR #3 ; R3 += (1/8)*(value<<16)
    
    MOV     R10, R3, LSR #16    ; 滤波值 = R3>>16 (Q15)
    SUBS    R11, R2, R4         ; div = 新值 - 前滤波值 (Q15)
    MOV     R4, R10             ; 更新前滤波值 (Q15)
    
    CMP     R11, R8             ; 比较div与Q15阈值
    IT      GT
    ADDGT   R5, R5, #1          ; upCount++ 
    BLE     else_branch1
back1

    ;--- 第2个数据 ---
    LDRH    R2, [R9, #2]        ; value[i+1]
    MOV     R10, R3, LSR #3
    SUB     R3, R3, R10
    LSL     R10, R2, #16
    ADD     R3, R3, R10, LSR #3
    MOV     R10, R3, LSR #16
    SUBS    R11, R2, R4
    MOV     R4, R10
    CMP     R11, R8
    IT      GT
    ADDGT   R5, #1
    BLE     else_branch2
back2

    ;--- 第3个数据 ---
    LDRH    R2, [R9, #4]        ; value[i+2]
    MOV     R10, R3, LSR #3
    SUB     R3, R3, R10
    LSL     R10, R2, #16
    ADD     R3, R3, R10, LSR #3
    MOV     R10, R3, LSR #16
    SUBS    R11, R2, R4
    MOV     R4, R10
    CMP     R11, R8
    IT      GT
    ADDGT   R5, #1
    BLE     else_branch3
back3

    ;--- 第4个数据 ---
    LDRH    R2, [R9, #6]        ; value[i+3]
    MOV     R10, R3, LSR #3
    SUB     R3, R3, R10
    LSL     R10, R2, #16
    ADD     R3, R3, R10, LSR #3
    MOV     R10, R3, LSR #16
    SUBS    R11, R2, R4
    MOV     R4, R10
    CMP     R11, R8
    IT      GT
    ADDGT   R5, #1
    BLE     else_branch4
back4

    ADD     R1, R1, #4          ; i += 4
    CMP     R1, #100
    BLT     loop_start
    B       loop_end

    ;===== ELSE分支处理 =====
else_branch1
    ADD     R6, R6, #1          ; downCount++
    CMP     R6, #2
    BLE     back1
    CMP     R5, #2
    IT      GT
    ADDGT   R7, R7, #1          ; pluseCount++
    MOV     R5, #0              ; 重置upCount
    B       back1

else_branch2
    ADD     R6, R6, #1          ; downCount++
    CMP     R6, #2
    BLE     back2
    CMP     R5, #2
    IT      GT
    ADDGT   R7, R7, #1
    MOV     R5, #0
    B       back2

else_branch3
    ADD     R6, R6, #1          ; downCount++
    CMP     R6, #2
    BLE     back3
    CMP     R5, #2
    IT      GT
    ADDGT   R7, R7, #1
    MOV     R5, #0
    B       back3

else_branch4
    ADD     R6, R6, #1          ; downCount++
    CMP     R6, #2
    BLE     back4
    CMP     R5, #2
    IT      GT
    ADDGT   R7, R7, #1
    MOV     R5, #0
    B       back4

    ;===== 循环结束 =====
loop_end
    ; 存储结果
    STRB    R5, [R0, #12]        ; upCount
    STRB    R6, [R0, #13]        ; downCount
    STRB    R7, [R0, #14]        ; pluseCount

    POP     {R4-R11, PC}         ; 恢复寄存器并返回
    ENDP





	ALIGN	4
    END		