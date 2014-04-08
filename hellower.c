#include <linux/kernel.h> /* Для printk() и т.д. */
#include <linux/module.h> /* Эта частичка древней магии, которая оживляет модули */
#include <linux/init.h> /* Определения макросов */
#include <linux/fs.h>
#include <asm/uaccess.h> /* put_user */
#include <linux/timer.h>

// Ниже мы задаём информацию о модуле, которую можно будет увидеть с помощью Modinfo
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Isachenko Andrew" );
MODULE_DESCRIPTION( "print hello in log every x second" );
MODULE_SUPPORTED_DEVICE("hellower");

#define SUCCESS 0
#define DEVICE_NAME "hellower" /* Имя нашего устройства */

// Поддерживаемые нашим устройством операции
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// Глобальные переменные, объявлены как static, воизбежание конфликтов имен.
static int major_number; /* Старший номер устройства нашего драйвера */
static int is_device_open = 0; /* Используется ли девайс ? */
static char text[] = "hello"; /* Текст, который мы будет отдавать при обращении к нашему устройству */
static char* text_ptr = text; /* Указатель на текущую позицию в тексте */

static int tick_time;
static int hello_counter;

// Прописываем обработчики операций на устройством
static struct file_operations fops =
{
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

//таймер
static struct timer_list my_timer;

void my_timer_callback(unsigned long data)
{
    if (tick_time != 0) {
        printk("%s: %d\n", text, hello_counter++);
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(tick_time));
    }
}

// Функция загрузки модуля. Входная точка. Можем считать что это наш main()
static int __init test_init(void)
{
	tick_time = 0;
	hello_counter = 0;
    printk( KERN_ALERT "TEST driver loaded!\n" );

 // Регистрируем устройсво и получаем старший номер устройства
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0) {
        printk("Registering the character device failed with %d\n", major_number);
        return major_number;
    } 
    //установка таймера
    setup_timer(&my_timer, my_timer_callback, 0);

 // Сообщаем присвоенный нам старший номер устройства
    printk("Test module is loaded!\n");

    printk("Please, create a dev file with 'mknod /dev/hellower c %d 0'.\n", major_number);

    return SUCCESS;
}

// Функция выгрузки модуля
static void __exit test_exit(void)
{
	//удаление таймера
	del_timer(&my_timer);
 	// Освобождаем устройство
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "Test module is unloaded!\n");
}

// Указываем наши функции загрузки и выгрузки
module_init(test_init);
module_exit(test_exit);

static int device_open(struct inode *inode, struct file *file)
{
    text_ptr = text;

    if ( is_device_open )
        return -EBUSY;

    is_device_open++;

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    is_device_open--;
    return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    sscanf(buff, "%d", &tick_time);
    tick_time *= 1000;
  	mod_timer(&my_timer, jiffies + msecs_to_jiffies(tick_time));
    return len;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) 
{
    int byte_read = 0;
    if (*text_ptr == 0 ) {
        return 0;
    }
    while (length && *text_ptr) {
        put_user(*(text_ptr++), buffer++);
        length--;
        byte_read++;
    }
    return byte_read;
}