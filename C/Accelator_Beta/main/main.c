#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "main";
#define MMA8452Q_ADDR 0x1C
#define TIME_OUT_MS 1000
#define DELAY_MS 1000

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21

#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

#define MMA8452Q_REG_OUT_X_MSB 0x01
#define MMA8452Q_REG_OUT_X_LSB 0x02
#define MMA8452Q_REG_OUT_Y_MSB 0x03
#define MMA8452Q_REG_OUT_Y_LSB 0x04
#define MMA8452Q_REG_OUT_Z_MSB 0x05
#define MMA8452Q_REG_OUT_Z_LSB 0x06
#define WHO_AM_I 0x0D
#define MMA8452Q_REG_CTRL_REG1 0x2A
#define MMA8452Q_REG_CTRL_REG2 0x2B
#define MMA8452Q_REG_CTRL_REG3 0x2C

#define FULL_SCALE_2G 0x00

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} mma8452q_data_t;

static esp_err_t i2c_master_read_slave_reg(i2c_port_t i2c_num, uint8_t i2c_addr,
                                           uint8_t i2c_reg, uint8_t *data_rd,
                                           size_t size) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_addr << 1), ACK_CHECK_EN);
  i2c_master_write_byte(cmd, i2c_reg, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_addr << 1) | READ_BIT, ACK_CHECK_EN);
  if (size > 1) {
    i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
  }
  i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin(i2c_num, cmd, TIME_OUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

static esp_err_t i2c_master_write_slave_reg(i2c_port_t i2c_num,
                                            uint8_t i2c_addr, uint8_t i2c_reg,
                                            uint8_t *data_wr, size_t size) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, i2c_reg, ACK_CHECK_EN);
  i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin(i2c_num, cmd, TIME_OUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

esp_err_t rdMMA8452Q(uint8_t i2c_reg, uint8_t *pdata, uint8_t count) {
  return i2c_master_read_slave_reg(I2C_NUM_1, MMA8452Q_ADDR, i2c_reg, pdata,
                                   count);
}

esp_err_t wrMMA8452Q(uint8_t i2c_reg, uint8_t *pdata, uint8_t count) {
  return i2c_master_write_slave_reg(I2C_NUM_1, MMA8452Q_ADDR, i2c_reg, pdata,
                                    count);
}

// JEDYNE CO NIE DZIAŁA TO FIX
static void mma8452q_init() {
  uint8_t data;
  wrMMA8452Q(MMA8452Q_REG_CTRL_REG1, &data, 1);
  data = 0x01;
  wrMMA8452Q(MMA8452Q_REG_CTRL_REG1, &data, 1);
  data = 0x00;
  wrMMA8452Q(MMA8452Q_REG_CTRL_REG2, &data, 1);
  data = 0x00;
  wrMMA8452Q(MMA8452Q_REG_CTRL_REG3, &data, 1);
}

static void i2c_master_innit() {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 100000;
  i2c_param_config(I2C_NUM_1, &conf);
  i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
}

uint16_t byte_swap(uint16_t val) { return (val >> 8) | (val << 8); }

static void i2c_test_task(void *arg) {
  esp_err_t err;
  mma8452q_data_t data;
  ESP_LOGI(TAG, "Starting MMA8452Q test");
  while (1) {
    err = rdMMA8452Q(MMA8452Q_REG_OUT_X_MSB, (uint8_t *)&data, sizeof(data));
    data.x = byte_swap(data.x);
    data.y = byte_swap(data.y);
    data.z = byte_swap(data.z);

    data.x /= 4;
    data.y /= 4;
    data.z /= 4;

    ESP_LOGI(TAG, "Accelerometr err:%d X: %d, Y: %d, Z: %d", err, data.x,
             data.y, data.z);
    vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
  }
}

void app_main(void) {

  i2c_master_innit();
  mma8452q_init();
  xTaskCreate(i2c_test_task, "i2c_test_task", 1024 * 2, NULL, 10, NULL);
}
