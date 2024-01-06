#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>


#define I2C_BUS_AVAILABLE 1
#define DEVICE_NAME "DS1307"
#define DS1307_ADDRESS 0x68
#define secAddress 0x00
#define dateAddress 0x04

static struct i2c_adapter *i2c_adapter = NULL; //i2c adapter
static struct i2c_client *i2c_client = NULL; //i2c client

static int i2c_write(unsigned char *buffer, unsigned int size){
    int ret = i2c_master_send(i2c_client, buffer, size);
    return ret;
}

static int i2c_read(unsigned char *buffer, unsigned int size){
    int ret = i2c_master_recv(i2c_client, buffer, size);
    return ret;
}

static void DS1307_write(unsigned char add, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char size){
    int ret;
    unsigned char buffer[8];
    buffer[0] = add;
    buffer[1] = data1;
    buffer[2] = data2;
    buffer[3] = data3;
    ret = i2c_write(buffer, size + 1);

}

static void DS1307_read(unsigned char *buffer, unsigned char size){
    int ret;
    ret = i2c_read(buffer, size);

}

static void DS1307_set_time(unsigned char hour, unsigned char min, unsigned char sec){
    DS1307_write(secAddress, sec, min, hour, 3);

}

static void DS1307_set_date(unsigned char day, unsigned char month, unsigned char year){
    DS1307_write(dateAddress, day, month, year, 3);

}

static void DS1307_get_time(unsigned char *hour, unsigned char *min, unsigned char *sec){
    unsigned char h_tmp, m_tmp, s_tmp;
    int ret;
    //set write
    DS1307_write(secAddress, 0, 0, 0, 0);
    unsigned char buffer[3];
    //read data
    DS1307_read(buffer, 3);
    s_tmp = buffer[0];
    m_tmp = buffer[1];
    h_tmp = buffer[2];

    s_tmp &= 0x7F;
    *sec = (s_tmp >> 4)*10 + (s_tmp&0x0F);
    m_tmp &= 0x7F;
    *min = (m_tmp >> 4)*10 + (m_tmp&0x0F);
    if(h_tmp & 0x40){
        h_tmp &= 0x1F;
        *hour = (h_tmp >> 4)*10 + (h_tmp&0x0F);

    } else
    {
        h_tmp &= 0x3F;
        *hour = (h_tmp >> 4)*10 + (h_tmp&0x0F);
    }
    

}

static void DS1307_get_date(unsigned char *day, unsigned char *month, unsigned char *year){
    int ret;
    //set write
    DS1307_write(dateAddress, 0, 0, 0, 0);
    unsigned char buffer[3];
    //read data
    DS1307_read(buffer, 3);
    *day = buffer[0];
    *month = buffer[1];
    *year = buffer[2];

    *day &= 0x3F;
    *day = (*day >> 4)*10 + (*day&0x0F);
    *month &= 0x1F;
    *month = (*month >> 4)*10 + (*month&0x0F);
    *year = (*year >> 4)*10 + (*year&0x0F);

}


static int etx_ds1307_probe(struct i2c_client *client, const struct i2c_device_id *id){
    unsigned char sec, min, hour, day, month, year;
    DS1307_get_time(&hour, &min, &sec);
    DS1307_get_date(&day, &month, &year);

    printk("Time: %d:%d:%d\n", hour, min, sec);
    printk("Date: %d:%d:%d\n", day, month, year);
    return 0; // hoặc giá trị phù hợp với việc kiểm tra thiết bị và khởi tạo, 0 thường được sử dụng khi không có lỗi
}


static void etx_ds1307_remove(struct i2c_client *client){
    printk("i2c was removed\n");
    // Các thao tác xử lý khi gỡ bỏ thiết bị i2c
}


static const struct i2c_device_id etx_ds1307_id[] = {
    {DEVICE_NAME, 0},
    {}
};


MODULE_DEVICE_TABLE(i2c, etx_ds1307_id);

static struct i2c_driver etx_ds1307_driver = {
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    .probe = etx_ds1307_probe,
    .remove = etx_ds1307_remove,
    .id_table = etx_ds1307_id,
};

static struct i2c_board_info ds1307_i2c_board_info = {
    I2C_BOARD_INFO(DEVICE_NAME, DS1307_ADDRESS)
};


static int __init driver_simple_init(void) {
    printk("Hello kernel\n");
    int ret = -1;
    i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if(i2c_adapter != NULL){
        i2c_client = i2c_new_client_device(i2c_adapter, &ds1307_i2c_board_info);
        if(i2c_client != NULL){
            i2c_add_driver(&etx_ds1307_driver);
            ret = 0;
        }
        i2c_put_adapter(i2c_adapter);
    }

    printk("Driver was added\n");
     return 0;
}


static void __exit driver_simple_exit(void) {
    printk("Goodbye kernel\n");
    i2c_unregister_device(i2c_client);
    i2c_del_driver(&etx_ds1307_driver);
    printk("Driver was removed\n");

}

module_init(driver_simple_init);
module_exit(driver_simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phan Hao");
MODULE_VERSION("1.0");