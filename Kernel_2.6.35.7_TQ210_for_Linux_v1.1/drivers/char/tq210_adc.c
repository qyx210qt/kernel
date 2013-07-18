/*************************************************************************************************
* drivers/char/tq210_ADC.c
* 功能简要： 
*	该驱动注册一个字符设备“/dev/EmbedSky_adc”, 用于实验ADC。
* 提供的接口：
*       ioctol(struct inode *inode,struct file *file,unsigned long arg);
*	用于选择通道和设置数据位，由命令来决定更新哪个值。 cmd =1 ,代表更新 通道。 cmd=2,代表更新 数据位。
*	这里只能选择 0==>AN0,1==>AN1,2==>AN2,3==>AN3 通道，
*	其他通道预留给LCD；数据位只能选择 12 或者 10
*	设置的通道值或者数据位的值不在该范围内，这保持上一次的数值，不作任何改变。
****************************************************************************************************/

#include <linux/input.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <plat/regs-adc.h>
#include <plat/adc.h>
#include <mach/map.h>

#define DEVICE_NAME    "tq210-adc"
#define PORT_SELECTED 1
#define BIT_SELECTED  3

struct tq210adc {
	struct s3c_adc_client *client;
	struct clk	*adc_clock;
	void __iomem 	*base_addr;
	int  adc_port;
	int  adc_data;
};

static struct tq210adc _adc;

static void tq210_adc_select(struct s3c_adc_client *client,
				unsigned select)
{
}
/**
 * tq210_adc_conversion - ADC conversion callback
 * @client: The client that was registered with the ADC core.
 * @data0: The reading from ADCDAT0.
 * @data1: The reading from ADCDAT1.
 * @left: The number of samples left.
 *
 * Called when a conversion has finished.
 */
static void tq210_adc_conversion(struct s3c_adc_client *client,
				  unsigned data0, unsigned data1,
				  unsigned *left)
{
	printk("%s: %d,%d\n", __func__, data0, data1);
}


	

/*Open函数，打开设备的接口*/
static int tq210_adc_open(struct inode *inode, struct file *file)
{
#ifdef CONFIG_TQ210_DEBUG_ADC
	printk(KERN_INFO "tq210_adc_open() entered\n");	
#endif
	
	return 0;
}


/*关闭设备的接口*/
static int tq210_adc_close(struct inode *inode, struct file *file)
{
	return 0;
}


/*
*这里采取轮询的方式，不停的检测ADCCON[15]的转换换成位来判断是否完成一次转换
*如果完成了转换，通过判断读取的数据位来读取数据，并件数据拷贝到用户空间数组
*/
static ssize_t  tq210_adc_read(struct file *file, char __user * buffer,size_t size, loff_t * pos)
{

	_adc.adc_data=s3c_adc_read(_adc.client, _adc.adc_port);

#ifdef CONFIG_TQ210_DEBUG_ADC
	printk("_adc.date==%d\n",_adc.adc_data);
#endif
	/*将读取到的AD转换后的值发往到上层应用程序*/
	if(copy_to_user(buffer, (char *)&_adc.adc_data, sizeof(_adc.adc_data)))
	{
		printk("copy data to user space failed !\n");
		return -EFAULT;		
	}

	return sizeof(_adc.adc_data);
}

/*
*ioctl 接口用于修改通道或者转换的数据位
*1 表示要修改通道
*2 表示要修改转换的数据位
*/
static long tq210_adc_ioctl(struct file *file,unsigned int cmd, unsigned long arg)//struct inode *inode, 
{
	unsigned int temp= (unsigned int)arg;
	printk(KERN_INFO " s3c_adc_ioctl(cmd:: %d) bit: %d \n", cmd, temp);

	switch (cmd)
	{
	case PORT_SELECTED://改变通道命令		
		if (temp >= 4)
		{
			printk(" %d is already reserved for TouchScreen\n", _adc.adc_port);
		}
		else
		{
			_adc.adc_port = temp;
		}
		return 0;
	case BIT_SELECTED://改变数据位命令
		if(temp ==12 || temp ==10)
		{
			_adc.client->data_bit=temp;
		}
		else
		{
			printk("nice guy,data bits should be 12 or 10 !\n");
			printk("%d-bit \n",temp);
		}
		return 0;
	default:
		printk("unknowed cmd :%d!\n", cmd);
		return -ENOIOCTLCMD;
	}
}

/*接口函数设置*/
static struct file_operations s3c_adc_fops = 
{
	.owner			= THIS_MODULE,
	.read			= tq210_adc_read,
	.open			= tq210_adc_open,
	.release 		= tq210_adc_close,
	.unlocked_ioctl	= tq210_adc_ioctl,
};

/*设备结构设置*/
static struct miscdevice s3c_adc_miscdev = 
{
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEVICE_NAME,
	.fops		= &s3c_adc_fops,
};

static char banner[] __initdata = KERN_INFO "TQ210 ADC driver\n";
/*
*设备初始化
*包括：虚拟地址的申请；平台时钟的获取；以及设备结构的注册
*如果成功:返回0
＊如果失败:注销申请的虚拟空间；注销时钟；
*/
int __init tq210_adc_init(void)
{
	int ret=0;	

	printk(banner);
	_adc.base_addr = ioremap(S5PV210_PA_ADC, 0x20);//SZ_4K

	if(_adc.base_addr ==  NULL){
		ret = -ENOENT;
		goto err_map;
	}
	_adc.adc_clock = clk_get(NULL, "adc");

	if(IS_ERR(_adc.adc_clock)){
		printk("failed to fine ADC clock source: %s \n",KERN_ERR);
		goto err_clk;
	}

	clk_enable(_adc.adc_clock);	

	ret = misc_register(&s3c_adc_miscdev);

	if (ret) {
		printk(DEVICE_NAME "can't register major number\n");
		goto err_clk;
	}

	_adc.client = kzalloc(sizeof(struct s3c_adc_client), GFP_KERNEL);

	if (!_adc.client) {
		printk("no memory for adc client\n");
		goto err_clk;
	}

	_adc.client->is_ts = 0;
	_adc.client->select_cb = tq210_adc_select;
	_adc.client->convert_cb = tq210_adc_conversion;
	_adc.adc_port=0;
	_adc.client->data_bit=12;

	printk(KERN_INFO "TQ210 ADC driver successfully probed\n");

	return 0;

err_clk:
	clk_disable(_adc.adc_clock);
	clk_put(_adc.adc_clock);

err_map:
	iounmap(_adc.base_addr);

	return ret;
}
/*
*注销模块接口函数
*ADC时钟的注销，设备结构的注销
*
*/
static void __exit tq210_adc_exit(void)
{
	iounmap(_adc.base_addr); /*释放虚拟地址映射空间*/

	if (_adc.adc_clock)   /*屏蔽和销毁时钟*/
	{
		clk_disable(_adc.adc_clock);    
		clk_put(_adc.adc_clock);
		_adc.adc_clock = NULL;
	}
	misc_deregister(&s3c_adc_miscdev);//*注销mi
}

module_init(tq210_adc_init);
module_exit(tq210_adc_exit);

MODULE_AUTHOR("www.embedsky.net");
MODULE_DESCRIPTION("TQ210 ADC driver");
MODULE_LICENSE("GPL");
