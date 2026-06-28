/********************************************************************************
    FileName    :  s_sensor.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  各个传感器的检测与保护

    Date        :  2018-10-17
    Modify      :
                   2018-10-17 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/
#include    "printMessage.h"
#include    <stdlib.h>
#include    <stdio.h>

//uint16_t 		buff[2046];			//预分配空间，
#ifdef	PrintMessage	

#define			MessageBuffSize		4*500

#else
#define			MessageBuffSize		4*2
#endif

uint16_t 		messgeOutBuff[MessageBuffSize];

uint16_t* memPoint=messgeOutBuff;				//得到信息区首址			


MessageDef  message;
// uint16_t* 		messageBuff[10];			//PAN时用两个150 当300缓存用

MessageBuffDef  MessageBuff[10];
MessageBuffDef  paraBuff;



static uint8_t messageLock=0;

uint8_t printMessagePop(MessageBuffDef* message,MessageBuffDef* messageBuff)     //将消息弹出到输出缓存
{ 
		uint16_t size=message->size;
        messageBuff->size=size;
		messageBuff->buff=memPoint;

		if(memPoint+size>messgeOutBuff+MessageBuffSize*sizeof(uint16_t))//缓存溢出
		{
			return 0;
		}	

		


        if(message->buff!=0)
        {

            for(uint16_t j=0;j<size;j++)
            {

                messageBuff->buff[j]=((uint16_t*) message->buff)[j]; 

            }
						memPoint+=size;			//指向下一空闭区首址
					
        }

        return size;

}




uint8_t    PrintMessagePush(MessageDef messageIn)   //将需要打印的数据压到堆里
{
	uint8_t ret=0;
#ifdef	PrintMessage	
	uint16_t size=messageIn.array[0].size;

    if(messageLock)
    {
        return ret;
    }
    messageLock=1;
    message=messageIn;




	uint8_t messageIndex=0;             //消息索引

		memPoint=messgeOutBuff;				//得到信息区首址		
        for(int i=0;i<PRINT_MESSAGE_BUFF_SIZE;i++)
        { 
		   ret=printMessagePop(&message.array[i],&MessageBuff[i]);
           if(ret==0) 
            {return ret;}

        }

        ret= printMessagePop(&message.paraArray,&paraBuff);       //参数区输出到缓存区
           if(ret==0) 
            {return ret;}
     

#endif    

		return ret;
}


uint8_t     PrintMessageFun(MessageBuffDef *messageBuff,uint16_t num)
{
    uint16_t   size=messageBuff->size;
    uint16_t   paraValue;  

    for (uint16_t i = 0; i < size; i++) 
    {
        printf("%d",i);
        for(uint8_t j=0;j<num;j++)
        {
            if((messageBuff+j)->size!=0)
            {
                paraValue=(messageBuff+j)->buff[i];
								printf("\t%d",paraValue);
            }
    
        }
				printf("\r\n");
				

    }

    return 1;

}

uint8_t     PrintMessageOut(void)
{
#ifdef	PrintMessage		
		if(messageLock==0)
		{
			return 0;
		}	
	
		if(message.num==PAN_MESSAGE)
		{
				printf("pan pluse is %d.\r\n",paraBuff.buff[0]); 
				printf("Index,para1,para2\r\n");  // CSV标题
		}
		else
		{


			printf("para0 is %d.phase Angle is %d,lowOn is %d,phaseUpHrtim is %d \r\n",   paraBuff.buff[0],   paraBuff.buff[1],   paraBuff.buff[2],   paraBuff.buff[3]); 
		  printf("Index,para1,para2,para3,para4\r\n");  // CSV标题

		}
		

        if(MessageBuff[0].size)
        {
            PrintMessageFun(&MessageBuff[0],4);          //打印消息 4个长消息
        }    
        if(MessageBuff[4].size)
        {
					printf("\r\n Index,para1,para2,para3,para4\r\n");  // CSV标题					
					
            PrintMessageFun(&MessageBuff[4],4);          //打印消息 4个短消息
        }




    // uint16_t size=message.size;
    // uint16_t   para[4]={0};   

    // for (uint16_t i = 0; i < size; i++) 
    // {
        
    //     for(uint8_t j=0;j<message.paraSize;j++)
    //     {
    //         if(messageBuff[j]!=0)
    //         {
    //             para[j]=messageBuff[j][i];
    //         }
    
    //     }
	// 			if(message.num==PAN_MESSAGE)
	// 			{
	// 			    printf("%d,%d,%d,\r\n", i,  para[0],para[1]);
	// 			}
	// 			else
	// 			{
	// 	            printf("%d,%d,%d,%d,%d\r\n", i,  para[0],para[1],para[2],para[3]);

	// 			}				
				
				

    // }

    // uint16_t size=message.size1;
    // uint16_t   para[4]={0}; 		
    // for (uint16_t i = 0; i < size1; i++) 
    // {
        
    //     for(uint8_t j=0;j<message.paraSize;j++)
    //     {
    //         if(messageBuff[j]!=0)
    //         {
    //             para[j]=messageBuff[j][i];
    //         }
    
    //     }



//		free(messageBuff[0]);
//		free(messageBuff[1]);
//		free(messageBuff[2]);
//		free(messageBuff[3]);
//		free(paraBuff);
		memPoint=messgeOutBuff;				//清空空间

    messageLock=0;
#endif
		
		return	1;			//打印输出完成
}