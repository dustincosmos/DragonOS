# MMIO

MMIO是“内存映射IO”的缩写，它被广泛应用于与硬件设备的交互之中。

## 地址空间管理

DragonOS中实现了MMIO地址空间的管理机制，本节将介绍它们。

### 为什么需要MMIO地址空间自动分配？

&emsp;&emsp;由于计算机上的很多设备都需要MMIO的地址空间，而每台计算机上所连接的各种设备的对MMIO地址空间的需求是不一样的。如果我们为每个类型的设备都手动指定一个MMIO地址，会使得虚拟地址空间被大大浪费，也会增加系统的复杂性。并且，我们在将来还需要为不同的虚拟内存区域做异常处理函数。因此，我们需要一套能够自动分配MMIO地址空间的机制。

### 这套机制提供了什么功能？

- 为驱动程序分配4K到1GB的MMIO虚拟地址空间
- 对于这些虚拟地址空间，添加到VMA中进行统一管理
- 可以批量释放这些地址空间

### 这套机制是如何实现的？

&emsp;&emsp;这套机制本质上是使用了伙伴系统来对MMIO虚拟地址空间进行维护。在`mm/mm.h`中指定了MMIO的虚拟地址空间范围，这个范围是`0xffffa10000000000`开始的1TB的空间。也就是说，这个伙伴系统为MMIO维护了这1TB的虚拟地址空间。

### 地址空间分配过程

1. 初始化MMIO-mapping模块，在mmio的伙伴系统中创建512个1GB的`__mmio_buddy_addr_region`
2. 驱动程序使用`mmio_create`请求分配地址空间。
3. `mmio_create`对申请的地址空间大小按照2的n次幂进行对齐，然后从buddy中申请内存地址空间
4. 创建VMA，并将VMA标记为`VM_IO|VM_DONTCOPY`。MMIO的vma只绑定在`initial_mm`下，且不会被拷贝。
5. 分配完成

一旦MMIO地址空间分配完成，它就像普通的vma一样，可以使用mmap系列函数进行操作。

### MMIO的映射过程

&emsp;&emsp;在得到了虚拟地址空间之后，当我们尝试往这块地址空间内映射内存时，我们可以调用`mm_map`函数，对这块区域进行映射。

&emsp;&emsp;该函数会对MMIO的VMA的映射做出特殊处理。即：创建`Page`结构体以及对应的`anon_vma`. 然后会将对应的物理地址，填写到页表之中。

### MMIO虚拟地址空间的释放

&emsp;&emsp;当设备被卸载时，驱动程序可以调用`mmio_release`函数对指定的mmio地址空间进行释放。

&emsp;&emsp;释放的过程中，`mmio_release`将执行以下流程：

1. 取消mmio区域在页表中的映射。
2. 将释放MMIO区域的VMA
3. 将地址空间归还给mmio的伙伴系统。
