# 信号监视器 - 基于Qt的多输入源波形显示器

信号监视器可以通过读取串口、文件流等来源的字符流数据，将其中包含的数据点显示在窗口中。

支持FFT以分析频域信号。



## 使用说明

### 数据流格式

通过串口输入的数据或者控制字须以一个或数个空白字符分隔，如 `\n` `\t` `\0` 或空格符。

#### 有效数据

有效数据可以是以十进制、十六进制（ `0x起始` ）或科学计数法表示的 `double` 类型数据，格式化为字符串形式发送。

工作在等间隔模式时，每个有效数据均表示一个数据点的幅值。

#### 控制字

有效数据流必须以 `%START` 起始，以 `%STOP` 结束（若需要实时显示数据波形，则可以不发送 `%STOP` ）

数据点的时间间隔通过 `%t<TIME_SPACE>` 控制字发送
其中 `<>` 不存在于发送数据中，仅标识其中包括了一个变量; `TIME_SPACE` 为 `double` 类型，单位 `us` 。



#### Example

一个包含4个点的数据流可以如下：

`%START %t10 1  0.809016994374948  0.309016994374947  -0.309016994374947 %STOP`

这段数据流将在图表中连续绘制 `(0, 1)` `(10, 0.809016994374948)` `(20, 0.309016994374947)` `(30, -0.309016994374947)` 这几个点并使用折线将其连接。