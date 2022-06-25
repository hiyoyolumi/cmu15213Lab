最近申请完了开源之夏，在项目开工和期末考试之前这段心浮气燥的时间里面，闲来无事 就接着写 15213 的 lab 吧

bomblab 是考察汇编代码的一个 lab，在看完对应课程（[Machine-Level Programming II: Control](https://scs.hosted.panopto.com/Panopto/Pages/Viewer.aspx?id=fc93c499-8fc9-4652-9a99-711058054afb)）并略读完 CSAPP 上对应章节之后，我开始了这个 lab

对于之前从来没有接触过汇编代码的人来说，强烈建议在开始之前读一遍 CSAPP 第三章，这样在做 lab 的过程中至少不会一头雾水一看到看不懂的指令就想看博客看解析

------

[TOC]

# 前言

[lab源文件下载](http://csapp.cs.cmu.edu/3e/bomb.tar)

压缩包解压之后只有三个文件

![image-20220624193214585](/home/hiixfj/.config/Typora/typora-user-images/image-20220624193214585.png)

bomb 可执行文件并不是由 bomb.c 单独一个源文件编译产生的，而是还有其他的一些文件（被老师隐藏起来了），bomb.c 源文件中的代码比较简单，大家可以快速读一下，里面没有包含 "炸弹" 的内容，我们要拆的炸弹就是在 bomb 这个二进制文件里面

二进制文件我们无法理解，但是我们可以使用反汇编工具 objdump 来生成我们可以理解的汇编代码：

> $ objdump -d bomb > assemble

开始之前，熟记这张图会对这次 lab 非常重要（图自《深入理解计算机系统》）

![image-20220625154404528](/home/hiixfj/.config/Typora/typora-user-images/image-20220625154404528.png)



先看 bomb.c 

![image-20220624194111881](/home/hiixfj/.config/Typora/typora-user-images/image-20220624194111881.png)

这段代码说明我们执行 bomb 时，可以有两种输入方式，一种是 stdin 标准输入，另一种则是从文件中输入，一行一个问题，可以避免我们每次重复输入之前已经解决过的问题

# GDB简单使用

这里只放一些我在做这个 lab 时用到的一些 GDB 简单操作所查阅的博客，GDB 是一个非常强大的工具，深入学习需要参考其他博客

[查看地址处的内容](https://blog.csdn.net/haifeng_gu/article/details/73928545)

查看寄存器的值：

> (gdb) info registers

[实时查看汇编代码以及寄存器的值](https://blog.csdn.net/zhangjs0322/article/details/10152279)

> 本次 lab 中常用的是 layout regs

------



## phase_1

先看 phase_1 的汇编代码：

```assembly
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi	# 把一串地址存入 %esi 中
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>	# %rdi %rsi 作为参数
  400eee:	85 c0                	test   %eax,%eax		# 判断返回值是否为 0
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>	# 如果相等的话 跳转
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>	# 不相等 bomb
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq   
```

关于汇编的解释我放在了代码块中，自己需要按着代码的流程顺序阅读，看完代码之后，我们来看如何解题。

这道题要我们输入一个字符串，我们可以猜测 %rsi 中存的就是正确答案，%rdi 中存的就是我们输入的字符串，那么我们就要在程序运行时查看 0x402400 这块地址存放的字符串是什么，使用 GDB ：

> $ gdb bomb

先在 400ee9 处打一个断点（注意给地址打断点，地址前面要加个 "*" 号）：

> (gdb) b *0x400ee9

运行代码：

> (gdb) run

查看 0x402400 地址处的字符串：

> (gdb) x/s 0x402400

如图：

![image-20220624200013087](/home/hiixfj/.config/Typora/typora-user-images/image-20220624200013087.png)

可以知道：本题的答案是 "Border relations with Canada have never been better."

## phase_2

```assembly
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp	# 栈指针下移
  400f02:	48 89 e6             	mov    %rsp,%rsi	# 把栈指针传给 %rsi,并把 %rsi 作为参数传入函数，猜测可能是作为数组地址
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>	# 读六个数
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)	# 第一个数应该是 1
  400f0e:	74 20                	je     400f30 <phase_2+0x34>	# 相等的话跳转
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>	# 否则爆炸
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>	
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax			# 把 arr[0] 传给 %eax
  400f1a:	01 c0                	add    %eax,%eax				# eax += eax
  400f1c:	39 03                	cmp    %eax,(%rbx)				# 比较 arr[next] 和 arr[pre]
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>	#  arr[next] != arr[pre] 的话 bomb
  400f25:	48 83 c3 04          	add    $0x4,%rbx				# %rbx 向数组后移一位
  400f29:	48 39 eb             	cmp    %rbp,%rbx				# 查看数组是否遍历结束
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx			# 第下一个数的地址传给 %rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp			# 数组结束地址
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq   
```

```assembly
000000000040145c <read_six_numbers>:
  40145c:	48 83 ec 18          	sub    $0x18,%rsp
  401460:	48 89 f2             	mov    %rsi,%rdx		# 数组首地址传给 %rdx arr[0]
  401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx	# %rcx arr[1]
  401467:	48 8d 46 14          	lea    0x14(%rsi),%rax
  40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp) 	# %rsp+0x8 arr[5]	
  401470:	48 8d 46 10          	lea    0x10(%rsi),%rax
  401474:	48 89 04 24          	mov    %rax,(%rsp)		# %rsp arr[4]
  401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9	# %r9 arr[3]
  40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8	# %r8 arr[2]
  401480:	be c3 25 40 00       	mov    $0x4025c3,%esi	# %esi INPUT FORMAT 0X4025C3
  401485:	b8 00 00 00 00       	mov    $0x0,%eax		# %rdi INPUT STRING
  40148a:	e8 61 f7 ff ff       	callq  400bf0 <__isoc99_sscanf@plt>	# 从输入中格式化地读取，返回读取到的个数
  40148f:	83 f8 05             	cmp    $0x5,%eax		# need string_len > 5
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
  401494:	e8 a1 ff ff ff       	callq  40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp
  40149d:	c3                   	retq   
```

在函数 phase_2 中我们可以看到，这个函数要我们输入六个数字，第一个数字要求是 1, 否则 bomb，并且要求后一个数字是前一个数字的两倍，因此本题：

> 1 2 4 8 16 32

为了不要每次运行都麻烦的去复制粘贴 phase_1 的字符串，我们可以新建一个文件，用来存放每一题的代码：

![image-20220624203343034](/home/hiixfj/.config/Typora/typora-user-images/image-20220624203343034.png)

注意，每一行结束之后要回车一行，以示 EOF

这样的话，我们运行代码的方式就变成了：

> $ bomb ans

这之后我们要如何使用 GDB 进行调试呢？我们只需要把程序的运行参数加入 GDB 中：

> $ (gdb) set args ans

## phase_3

```assembly
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp		# 熟悉的 栈用作数组
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx	# %rcx arr[0]
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx	# %rdx arr[1]
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi	# esi INPUT FORMAT 0x4025cf
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>	# 可以猜测 输入两个数，存入数组中
  400f60:	83 f8 01             	cmp    $0x1,%eax		# need eax > 1
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)	# need arr[0] <= 7 && arr[0] >=0
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax	# eax = arr[0]
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>	# 为了不 bomb，我们要使 arr[1] == eax
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq   
```

这道题，通过前段的代码阅读，我们可以发现：要输入两个数，且第一个数必须属于 [0,7]，否则 bomb，至于第二个数，我们可以随机输入一个符合范围内的 arr[0]，再通过 GDB 来查看在 400fc2 处 %eax 的值，以此来得出答案:

![image-20220625150951185](/home/hiixfj/.config/Typora/typora-user-images/image-20220625150951185.png)

此例中，我输入的第一个数是 "5"，执行到 0x400fc2 处的时候，寄存器 %rax 的值为 206；这套题还有其他的答案组合，大家可以试一试（要记得第一个输入数的范围）

## phase_4

```assembly
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx	# 是不是很熟悉，和 phase_3 类似
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax		# 要输入两个数 arr[0] arr[1]
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)			# 输入的第一个数
  401033:	76 05                	jbe    40103a <phase_4+0x2e>	# 要求 arr[0] <= 14 && arr[0] >= 0
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx		# 第三个参数 14
  40103f:	be 00 00 00 00       	mov    $0x0,%esi		# 第二个参数 0
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi	# 第一个参数 arr[0]
  401048:	e8 81 ff ff ff       	callq  400fce <func4>	# 上面是函数 func4 的三个参数，不妨假设为 func(int a, int b, int c)
  40104d:	85 c0                	test   %eax,%eax		# 测试返回值
  40104f:	75 07                	jne    401058 <phase_4+0x4c>	# 要求 func4() 返回值为 0
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)			# 要求第二个输入数为 0
  401056:	74 05                	je     40105d <phase_4+0x51>	# 否则 bomb
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq   
```

这道题我们需要具体看看 func4(int a, int b, int c) 函数内部是如何实现的：

```assembly
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax			# eax = c
  400fd4:	29 f0                	sub    %esi,%eax			# eax -= b	(eax = c - b)
  400fd6:	89 c1                	mov    %eax,%ecx			# ecx = eax (ecx = c - b)
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx			#
  400fdb:	01 c8                	add    %ecx,%eax			# 判断 eax 的符号
  400fdd:	d1 f8                	sar    %eax					# eax /= 2 	(eax = (c - b) / 2)
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx	# ecx = rax + b (ecx = (b + c) / 2)
  400fe2:	39 f9                	cmp    %edi,%ecx			# compare (b + c) / 2 with a
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>	# if (ecx > a)
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx			# edx = rcx - 1 (c = (b + c) / 2 - 1)
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>			# func4(a, b, c) ( func(a, b, (b+c)/2-1) )
  400fee:	01 c0                	add    %eax,%eax				# rax += rax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>		# return rax
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax			# if (ecx <= a)	rax = 0
  400ff7:	39 f9                	cmp    %edi,%ecx				# compare (b + c) / 2 with a
  400ff9:	7d 0c                	jge    401007 <func4+0x39>		# if (ecx < a)
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi				# esi = rcx + 1 (b = (b + c) / 2 + 1)
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>				# func4(a, b, c)
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax		# rax = rax + rax + 1
  401007:	48 83 c4 08          	add    $0x8,%rsp				# if (ecx >= a)
  40100b:	c3                   	retq   								# return rax
```

这个函数还是比较绕的，不过在阅读代码的同时把注释写出来之后，就会好理解一些了，根据代码注释，我们可以得出这样一个代码：

```c
int func4(int input, int b, int c) {	// init input (input, 0, 14)
    int val = (b + c) / 2;
    if (val <= input) {
        if (val >= input) {
            return 0;
        }
        int res = 2 * func4(input, val + 1, c) + 1;
        return res;
    } else {
        int res = 2 * func4(input, b, val - 1);
        return res;
    }
}
```

编写测试代码，可以得出所有输出：

> 0 0
>
> 1 0
>
> 3 0
>
> 7 0

## phase_5

```assembly
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx					# %rbx = inputString
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
  401078:	31 c0                	xor    %eax,%eax					# 将 %eax 设置为 0
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>		# 得到输入字符串的长度
  40107f:	83 f8 06             	cmp    $0x6,%eax					# 要求输入字符串的长度为 6
  401082:	74 4e                	je     4010d2 <phase_5+0x70>		
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx           # 把输入字符串中第 rax 个字符取出来 放到 %ecx
  40108f:	88 0c 24             	mov    %cl,(%rsp)                   # 
  401092:	48 8b 14 24          	mov    (%rsp),%rdx                  # 这两行是把第 rax 个字符的 ASCII 码放到 %rdx
  401096:	83 e2 0f             	and    $0xf,%edx                    # 根据掩码 1111 来得到 ASCII 码的低四位 放到 %rdx
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx      	# 在 rdx + 0x4024b0 处取一个字符放到 %rdx
  																		# "maduiersnfotvbyl"
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)        # 把 %rdx 中存放的字符放到 rsp + rax + 0x10 处
  4010a4:	48 83 c0 01          	add    $0x1,%rax					# rax += 1
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax					
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>		# while (rax != 6) go loop!
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)				# 字符串结尾设置结束的 '\0'
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi               # 期望得到的值
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi              # 判断根据我们输入的字符串经过变换是否与期望的字符串相等
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>	# 可以猜测 strings_not_equal() 当字符串不相等时返回 0
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>		# if (inputString == expect) bomb!
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax					# rax = 0
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	retq   
```

这道题当时折磨了我好久，但是其实只要把循环体中的逻辑弄明白之后再看这道题，就会简单很多；

其实这道题的核心逻辑就是：

> - 依次取出输入字符串的每个字符
> - 得到这个字符 ASCII 码的 低四位
> - 以这个低四位的值作为偏移量，去地址 0x4024b0 处的字符串上面找对应偏移量对应的字符
> - 将这些经过转换后的字符串与期望字符串进行比较

那么首先我们需要知道题目的期望字符串，可以使用 GDB 得到：

> (gdb) x/s 0x40245e
>
> "flyers"

其次我们还需要知道 0x4024b0 处的值：

> (gdb) x/s 0x4024b0
>
> "maduiersnfotvbyl"

那么我们可以得到 "flyers" 每个字符的低四位（对应的偏移量）：

> 9 15 14 5 6 7

再通过查表，我们可以得到答案：

> i 0 n e f g
>
> y   ~ u v w

由于低四位无法准确确定一个 ASCII 中的字符，因此会有两行低四位对应的字符，这两行任意组合都是正确答案

------

phase_6 还没有看完呢，临近期末考试了，考完了再慢慢看......