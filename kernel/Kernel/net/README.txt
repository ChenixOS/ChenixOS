common目录
通用组件
介于协议栈和用户层之间提供API

NIPF目录 Network interface platform framework
网络接口平台框架

介于协议栈和网卡驱动之间，提供一个中间层，方便协议栈对后方的网卡驱动进行驱动
1、提供给协议栈接口，完成解耦
2、提供给网卡驱动接口，而无需关心协议栈的实现，也可以随时替换新的协议栈
3、提供/dev/nipf和/dev/tap0字符设备
4、提供/proc/net下的状态输出

lwip目录 Light weight IP stack
轻量化的TCP/IP协议,是瑞典计算机科学院(SICS)的Adam Dunkels 开发的一个小型开源的TCP/IP协议栈
因为移植性好，是目前暂定使用的协议栈
