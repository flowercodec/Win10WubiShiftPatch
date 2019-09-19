# Win10WubiShiftPatch

 “ Windows 10 禁用五笔输入 shift 切换” 解决方案  
注：只对特定ChsIME.exe 文件才有效果。  

满足条件：  
1.系统: Windows 10 64位 (32位系统的 ChsIME.exe 文件肯定不一样，不用尝试了)  
2. 中文输入法文件 ： C:\Windows\System32\InputMethod\CHS\ChsIME.exe  MD5 = b3448bf077665f2e1ca67094bcf2a7c5  


方案原理：  
修改内存，使五笔输入法的shift 快捷键无效  
1.起始地址 ： (ChsIME.exe 基址) + 0x14DE1  
2.修改内容 :  两个字节长度  0x31, 0xC0   

现成工具：  
需要管理员权限运行  
下载地址 : https://github.com/flowercodec/Win10WubiShiftPatch/releases  
开源代码 : https://github.com/flowercodec/Win10WubiShiftPatch  
注：只有md5匹配时，才会打patch  

FAQ  
1.如何恢复shift切换功能？  
关闭patch程序后。在Windows任务管理器，强杀 "ChsIME.exe" 进程，"ChsIME.exe" 会重新启动。这时，shift功能就恢复了。  

2.打了patch后，shfit禁了，怎么切换中英文?  
Ctrl + 空格，反正我是从来没习惯用 shift 切换。  

3.需要支持其它“型号”的ChsIME.exe怎么办？  
联系作者 QQ: 320581388，肯定是免费的，但是也要看作者是否有时间，作者没有义务24小时内处理，毕竟MS好几年都不修改。
