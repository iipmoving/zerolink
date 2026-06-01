# WaveCapture 数据格式说明 — 桌面端解码

> MODBUS 0x5000 区, FC03 读取, 115200 baud

## 1. 寄存器映射

| 地址 | 名称 | R/W | 说明 |
|------|------|-----|------|
| 0x5000 | ACK | W | Host 写任意值 = 确认读完, 解冻 buffer |
| 0x5001 | STATUS | R | bit0=READY (1=数据就绪), bit1=COLLECTING |
| 0x5002 | FRAME_ID | R | 批次自增 ID, Host 检测变化判断新数据 |
| 0x5003 | COUNT | R | 本批次帧数 (≤ MAX_FRAMES) |
| 0x5004 | DATA_WORDS | R | data[] 总占用字数 (含所有 size 前缀) |
| 0x5005 | MAX_FRAMES | R/W | 目标帧数, 默认 10 |
| 0x5006+ | DATA | R | 帧数据体, 自描述格式 |

## 2. DATA 自描述格式

```
DATA = [size0][帧0数据][size1][帧1数据]...[sizeN][帧N数据]

 sizeN: 1 word (uint16), 第 N 帧的总字数
 帧N数据: sizeN words, 内容 = 所有 array[0..m] 的原始数据 + paraArray
```

**关键特性**:
- 每帧独立, 大小不必相同 (PPG 可能在微调, size 可能变)
- `DATA_WORDS` = 总占用字数, Host 据此决定一次读多少寄存器
- 帧内布局顺序: `array[0]数据 | array[1]数据 | ... | paraArray`

## 3. 具体示例 — 50Hz 半周期采集

### 3.1 MCU 端配置

每 1ms 调用一次 `WaveCapture_PushMessage(&msg)`:

```c
MessageDef msg;

// 3 个数据源, 每源 50 个采样点
msg.array[0].buff = TxaHrtimBuff[ch];   // HRTIM 时间戳
msg.array[0].size = 50;
msg.array[1].buff = TxaVcBuff[ch];      // 母线电压 ADC
msg.array[1].size = 50;
msg.array[2].buff = TxaFmacBuff[ch];    // 谐振电流 ADC
msg.array[2].size = 50;

// 5 个参数 (PPG 占空比, highOn, power, period, 保留)
msg.paraArray.buff = para;
msg.paraArray.size = 5;

WaveCapture_PushMessage(&msg);
```

### 3.2 每帧大小

```
frame_words = 50(HRTIM) + 50(V) + 50(I) + 5(para) = 155 words
```

### 3.3 10 帧完整数据

```
Header (0x5000-0x5005):  6 words
Frame 0:  [155][HRTIM×50][V×50][I×50][para×5]   = 1+155 = 156 words
Frame 1:  [155][HRTIM×50][V×50][I×50][para×5]   = 1+155 = 156 words
...
Frame 9:  [155][HRTIM×50][V×50][I×50][para×5]   = 1+155 = 156 words

DATA_WORDS = 10 × 156 = 1560
总寄存器数 = 6(header) + 1560(data) = 1566
单次 FC03 最长 125 寄存器 → 需要 13 次批量读
```

### 3.4 帧内字节布局 (第 N 帧)

```
offset  content
  0     155          ← 本帧总字数 (uint16)
  1     HRTIM[0]     ← 第 1 个时间戳
  2     HRTIM[1]
 ...
 50     HRTIM[49]    ← 第 50 个时间戳
 51     V[0]         ← 第 1 个电压 ADC
 52     V[1]
 ...
100     V[49]
101     I[0]         ← 第 1 个电流 ADC
102     I[1]
 ...
150     I[49]
151     para[0]      ← PPG 占空比
152     para[1]      ← highOn
153     para[2]      ← power
154     para[3]      ← period
155     para[4]      ← 保留
```

## 4. Python 解码

```python
import struct
from pymodbus.client import ModbusSerialClient

def read_wave_capture(client, slave=5):
    """读取 0x5000 区完整波形数据"""
    # 1. 读 header
    hdr = client.read_holding_registers(0x5000, 6, device_id=slave)
    if hdr.isError():
        return None

    ack, status, frame_id, count, data_words, max_frames = hdr.registers

    if not (status & 0x01):  # READY?
        return None

    # 2. 读数据体
    data = client.read_holding_registers(0x5006, data_words, device_id=slave)
    if data.isError():
        return None

    raw = data.registers  # list of uint16

    # 3. 按自描述格式解析
    frames = []
    pos = 0
    for i in range(count):
        if pos >= len(raw):
            break
        fw = raw[pos]          # 本帧字数
        pos += 1
        frame_data = raw[pos:pos + fw]
        pos += fw

        # 已知布局: HRTIM[50] | V[50] | I[50] | para[5]
        frames.append({
            'hrtim':  frame_data[0:50],
            'voltage': frame_data[50:100],
            'current': frame_data[100:150],
            'para':   frame_data[150:155],
        })

    # 4. 确认读完
    client.write_register(0x5000, 1, device_id=slave)

    return {
        'frame_id': frame_id,
        'count': count,
        'frames': frames,
    }
```

## 5. MATLAB 解码

```matlab
function w = wavecapture_read(mb, slave)
    % mb: modbus object (Instrument Control Toolbox)
    % 返回 struct: w.frame_id, w.frames(i).hrtim, .voltage, .current, .para

    % 1. 读 header
    hdr = read(mb, 'holdingregs', 0x5000, 6, slave);
    status   = hdr(2);
    frame_id = hdr(3);
    count    = hdr(4);
    data_w   = hdr(5);

    if bitand(status, 1) == 0
        w = []; return;  % 未就绪
    end

    % 2. 读数据体
    raw = read(mb, 'holdingregs', 0x5006, data_w, slave);

    % 3. 解析
    pos = 1;
    for i = 1:count
        fw = raw(pos);
        pos = pos + 1;
        d = raw(pos:pos+fw-1);
        pos = pos + fw;

        w.frames(i).hrtim   = d(1:50);
        w.frames(i).voltage = d(51:100);
        w.frames(i).current = d(101:150);
        w.frames(i).para    = d(151:155);
    end
    w.frame_id = frame_id;
    w.count    = count;

    % 4. 确认读完
    write(mb, 'holdingregs', 0x5000, 1, slave);
end
```

## 6. Host 操作流程

```
[上电初始化]
  (可选) 写 0x5005 = 10           ← 设帧数, 默认 10

[轮询等待]
  loop:
    读 0x5001 (1 reg)             ← 检查 STATUS bit0
    if bit0 == 0: sleep(50ms); continue

[读取数据]
    FRAME_ID = 读 0x5002          ← 记下 frame_id, 下次对比用
    COUNT    = 读 0x5003 (1 reg)
    DATA_W   = 读 0x5004 (1 reg)
    raw      = 批量读 0x5006, DATA_W 字

[解析]
    按 size 前缀格式逐帧切分

[确认]
    写 0x5000 = 任意值            ← 解冻, 模块开始下一批采集
    GOTO [轮询等待]
```

## 7. 与 0x4000 区的关系

| 区 | 内容 | 频率 | 用途 |
|----|------|------|------|
| 0x4000 | f0, Q, L, I_peak, f_sw | 每周期 (~33μs) 更新 | 实时监控, 抬锅检测 |
| 0x5000 | 完整波形快照 | 每 10ms 冻结一批 | MATLAB 重分析, 模型验证 |

两者互补。0x4000 是抽炼后的特征值, 0x5000 是原始波形用于离线建模。

## 8. 传输时间

```
DATA_WORDS ≈ 1560 (10帧 × 156)
单次 FC03 最长 125 寄存器 → 186 bytes → 16ms @115200
需 13 次批量读 → 总传输约 210ms

推荐轮询间隔: 500ms~1s
```

## 9. 硬件参数参考

| 参数 | 值 | 用于换算 |
|------|-----|----------|
| V_div_ratio | 1/131.6 (810K/6.2K) | Vbus(V) = V_adc × 0.106 |
| CT_ratio | 2000:1 | I(A) = I_adc / 39.66 |
| R_burden | 2KΩ | — |
| C_resonant | 0.90μF (0.45×2) | f0 = 1/(2π√LC) |
| HRTIM_CLK | 768MHz | dt = ΔHRTIM / 768M |

详细换算见 `tools/ekf_tuner/calc_physical_params.m`。
