#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phan Hao");
MODULE_DESCRIPTION("A driver to write 'Hello World' to a LCD text display with I2C");

#define DRIVER_NAME "lcd_i2c"

/* LCD char buffer */
static char lcd_buffer[17];

/* I2C device structure */
static struct i2c_client *lcd_client;

/**
 * @brief Send a command to the LCD
 *
 * @param cmd: Command to send
 */
void lcd_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd}; // Control byte followed by command
    i2c_master_send(lcd_client, buf, sizeof(buf));
    msleep(5);
}

/**
 * @brief Send data to the LCD
 *
 * @param data: Data to send
 */
void lcd_data(uint8_t data) {
    uint8_t buf[2] = {0x40, data}; // Control byte followed by data
    i2c_master_send(lcd_client, buf, sizeof(buf));
    msleep(5);
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta, i;

    /* Get amount of data to copy */
    to_copy = min(count, sizeof(lcd_buffer));

    /* Copy data to user */
    not_copied = copy_from_user(lcd_buffer, user_buffer, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;

    /* Set the new data to the display */
    lcd_command(0x01); // Clear display

    for (i = 0; i < to_copy; i++)
        lcd_data(lcd_buffer[i]);

    return delta;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = driver_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
    struct i2c_adapter *adapter;

    printk("Hello, Kernel!\n");

    /* Detect the I2C adapter */
    adapter = i2c_get_adapter(1); // Use the appropriate I2C bus number

    /* Create a new I2C client */
    struct i2c_board_info info = {
        .type = "lcd1602",
        .addr = 0x27, // Địa chỉ I2C của LCD1602
    };

    lcd_client = i2c_new_client_device(adapter, &info);

    /* Initialize the LCD */
    msleep(50); // Wait for initialization

    lcd_command(0x38); // 2 lines, 5x7 matrix
    lcd_command(0x0C); // Display on, cursor off, blinking off
    lcd_command(0x01); // Clear display

    // Hiển thị chuỗi "Hello World" trên LCD
    const char *hello = "Hello World!";
    int hello_len = strlen(hello);
    driver_write(NULL, hello, hello_len, NULL);

    return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
    /* Clear the display */
    lcd_command(0x01);

    /* Delete the I2C client */
    i2c_unregister_device(lcd_client);

    printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);
