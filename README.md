# 信号监视器 - 基于Qt的多输入源波形显示器

信号监视器可以通过读取串口、文件流等来源的字符流数据，将其中包含的数据点显示在窗口中。

支持FFT以分析频域信号。

![Image text](https://raw.githubusercontent.com/nmpassthf/SignalMonitor/master/Documents/Assets/porgram_img.png)

## 使用说明

### 数据流格式

通过串口输入的数据或者控制字须以一个或数个空白字符分隔，如 `\n` `\t` `\0` 或空格符。

#### 有效数据

有效数据可以是以十进制、十六进制（ 起始为`0x` ）或科学计数法表示的 `double` 类型数据，**格式化为字符串**形式发送。

工作在等间隔模式时，每个有效数据均表示一个数据点的幅值。

#### 控制字

有效数据流必须以 `%START%` 起始，以 `%STOP%` 结束（若需要实时显示数据波形，则可以不发送 `%STOP`% ），区分大小写。

数据点的时间间隔通过 `%t<TIME_SPACE>%` 控制字发送。
其中 `<>` 不存在于发送数据中，仅标识其中包括了一个变量; `TIME_SPACE` 为 `double` 类型，单位 `us` 。

##### 当前可用的控制字如下表：

数据流起始： `%START% `

数据流结束： `%STOP%`

选择后续数据所绘制的子图号 ： `%SUBPLOT <INDEX>%`

​	 `INDEX` 为无符号整型，默认已选择为 `0` ，须连续增大（即发送 `%SUBPLOT 2` 必须先发送 `%SUBPLOT 1` ）

设置X轴自动步进的间隔： `%T <TIME_SPACE>%` ， `TIME_SPACE` 为 `double` 类型

设置X轴可见的范围（仅在启用示波器模式有效）： `%SETRANGE <RANGE>%` ， `RANGE` 为double类型

清除数据： `%CLEAR%` ，将清除前面已接收的所有数据，重置起始x坐标为 `0`

设置Y坐标轴使用对数轴： `%USELOGAXES%`

设置坐标轴名称： `%SETPLOTNAME <X_NAME>;<Y_NAME>%` ， `X_NAME` 和 `Y_NAME`  为不包含空白字符和 `;` 字符的字符串

设置坐标轴的单位： `%SETPLOTUNIT <X_UNIT>;<Y_UNIT>%` ，  `X_UNIT` 和 `Y_UNIT` 为不包含空白字符的字符串，若不修改某一坐标系，则将其值设置为 `{}`

当使用 `%SETPLOTUNIT%` 时，若指定单位为：`us ms s Rad Degree` 时，将自动转换显示格式。其中时间相关的单位将转换为 `HH:MM:SS-MS_US` 的形式，`Rad`

 将换算数值为 `<NUMBER> Pi` 显示， `Degree` 将显示标签为 `<NUMBER>°`

#### Example

一个包含4个点的数据流可以如下：

`%START% %SETRANGE 1024% %SETPLOTNAME Time;Amptitute% %SETPLOTUNIT s;V% %T 10% %SUBPLOT 0% 1  0.809016994374948  0.309016994374947  -0.309016994374947 %STOP%`

这段数据流将在图表中连续绘制 `(0, 1)` `(10, 0.809016994374948)` `(20, 0.309016994374947)` `(30, -0.309016994374947)` 这几个点并使用折线将其连接。