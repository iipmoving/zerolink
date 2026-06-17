/**
 * @file circular_queue.c
 * @brief 循环队列实现
 */

#include "queue.h"

/**
 * @brief 初始化队列
 */
void queue_init(CircularQueue *queue)
{
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->is_full = false;
}

/**
 * @brief A进程：向队列写入数据
 */
bool queue_write(CircularQueue *queue, uint8_t data)
{
    // 检查队列是否已满
    if (queue->is_full) {
        return false;
    }
    
    // 写入数据
    queue->buffer[queue->rear] = data;
    queue->rear++;
    queue->count++;
    
    // 检查是否到达缓冲区末尾
    if (queue->rear >= QUEUE_MAX_SIZE) {
        queue->rear = 0;  // 循环到开头
    }
    
    // 检查队列是否已满
    if (queue->rear == queue->front && queue->count > 0) {
        queue->is_full = true;
    }
    
    return true;
}

/**
 * @brief A进程：批量写入数据
 */
uint16_t queue_write_batch(CircularQueue *queue, uint8_t *data, uint16_t length)
{
    uint16_t written = 0;
    
    for (uint16_t i = 0; i < length; i++) {
        if (!queue_write(queue, data[i])) {
            break;  // 队列已满，停止写入
        }
        written++;
    }
    
    return written;
}

/**
 * @brief B进程：从队列读取数据
 */

uint8_t queue_read_one(CircularQueue *queue)
{
    // 检查队列是否为空
    if (queue->count == 0 && !queue->is_full) {
        return 0;
    }
    
    // 读取数据
    uint8_t xRet = queue->buffer[queue->front];
    queue->front++;
    queue->count--;
    queue->is_full = false;  // 读取后队列肯定不满
    
    // 检查是否到达缓冲区末尾
    if (queue->front >= QUEUE_MAX_SIZE) {
        queue->front = 0;  // 循环到开头
    }
    
    return xRet;
}
	
	



bool queue_read(CircularQueue *queue, uint8_t *data)
{
    // 检查队列是否为空
    if (queue->count == 0 && !queue->is_full) {
        return false;
    }
    
    // 读取数据
    *data = queue->buffer[queue->front];
    queue->front++;
    queue->count--;
    queue->is_full = false;  // 读取后队列肯定不满
    
    // 检查是否到达缓冲区末尾
    if (queue->front >= QUEUE_MAX_SIZE) {
        queue->front = 0;  // 循环到开头
    }
    
    return true;
}

/**
 * @brief B进程：批量读取数据
 */
uint16_t queue_read_batch(CircularQueue *queue, uint8_t *buffer, uint16_t max_length)
{
    uint16_t read_count = 0;
    
    for (uint16_t i = 0; i < max_length; i++) {
        if (!queue_read(queue, &buffer[i])) {
            break;  // 队列为空，停止读取
        }
        read_count++;
    }
    
    return read_count;
}

/**
 * @brief B进程：读取数据但不移动指针
 */
bool queue_peek(CircularQueue *queue, uint8_t *data)
{
    // 检查队列是否为空
    if (queue->count == 0 && !queue->is_full) {
        return false;
    }
    
    // 读取数据但不移动指针
    *data = queue->buffer[queue->front];
    return true;
}

/**
 * @brief B进程：重置队列
 */
void queue_reset(CircularQueue *queue)
{
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->is_full = false;
}

/**
 * @brief 获取队列当前数据个数
 */
uint16_t queue_get_count(CircularQueue *queue)
{
    return queue->count;
}

/**
 * @brief 检查队列是否为空
 */
bool queue_is_empty(CircularQueue *queue)
{
    return (queue->count == 0 && !queue->is_full);
}

/**
 * @brief 检查队列是否已满
 */
bool queue_is_full(CircularQueue *queue)
{
    return queue->is_full;
}

/**
 * @brief 获取队列剩余空间
 */
uint16_t queue_get_free_space(CircularQueue *queue)
{
    if (queue->is_full) {
        return 0;
    }
    return QUEUE_MAX_SIZE - queue->count;
}





