#include "sy210.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace sy210 {

static const char *const TAG = "sy210";

// SY210 协议常量
static const uint8_t SY210_HEADER1 = 0x42;
static const uint8_t SY210_HEADER2 = 0x4D;
static const size_t SY210_FRAME_SIZE = 5;
static const size_t MAX_BUFFER_SIZE = 64;

void SY210Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SY210 sensor...");
  this->buffer_.clear();
  this->buffer_.reserve(MAX_BUFFER_SIZE);
}

void SY210Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "SY210 Sensor:");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "PM2.5", this->pm25_sensor_);
  LOG_SENSOR("  ", "PM10", this->pm10_sensor_);
  LOG_SENSOR("  ", "PM1.0", this->pm1_0_sensor_);
  this->check_uart_settings(9600);
}

void SY210Sensor::update() {
  ESP_LOGV(TAG, "Requesting update...");
  this->process_data();
}

void SY210Sensor::clear_buffer() {
  this->buffer_.clear();
  ESP_LOGV(TAG, "Buffer cleared");
}

void SY210Sensor::process_data() {
  // 读取所有可用数据到缓冲区
  while (this->available()) {
    uint8_t byte;
    if (this->read_byte(&byte)) {
      this->buffer_.push_back(byte);
      
      // 防止缓冲区过大
      if (this->buffer_.size() > MAX_BUFFER_SIZE) {
        ESP_LOGW(TAG, "Buffer overflow (%zu bytes), clearing", this->buffer_.size());
        this->clear_buffer();
        break;
      }
    } else {
      break;
    }
  }
  
  ESP_LOGV(TAG, "Buffer size: %zu bytes", this->buffer_.size());
  
  // 查找并处理完整的帧
  while (this->buffer_.size() >= SY210_FRAME_SIZE) {
    // 查找帧头
    size_t header_pos = SIZE_MAX;
    
    for (size_t i = 0; i <= this->buffer_.size() - SY210_FRAME_SIZE; i++) {
      if (this->buffer_[i] == SY210_HEADER1 && i + 1 < this->buffer_.size() && 
          this->buffer_[i + 1] == SY210_HEADER2) {
        header_pos = i;
        break;
      }
    }
    
    if (header_pos == SIZE_MAX) {
      // 没找到完整帧头，保留最后一个字节（可能是帧头的开始）
      if (this->buffer_.size() > 1) {
        if (this->buffer_.back() == SY210_HEADER1) {
          uint8_t last_byte = this->buffer_.back();
          this->buffer_.clear();
          this->buffer_.push_back(last_byte);
        } else {
          this->buffer_.clear();
        }
      }
      break;
    }
    
    // 移除帧头之前的无用数据
    if (header_pos > 0) {
      ESP_LOGV(TAG, "Removing %zu bytes before frame header", header_pos);
      this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + header_pos);
    }
    
    // 检查是否有完整帧
    if (this->buffer_.size() < SY210_FRAME_SIZE) {
      ESP_LOGV(TAG, "Incomplete frame, waiting for more data");
      break;
    }
    
    // 解析帧
    if (this->parse_frame(this->buffer_.data(), SY210_FRAME_SIZE)) {
      ESP_LOGV(TAG, "Frame parsed successfully");
      // 成功解析，移除这个帧
      this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + SY210_FRAME_SIZE);
    } else {
      ESP_LOGV(TAG, "Frame parsing failed, removing first byte");
      // 解析失败，移除第一个字节继续查找
      this->buffer_.erase(this->buffer_.begin());
    }
  }
}

bool SY210Sensor::parse_frame(const uint8_t *data, size_t length) {
  if (length < SY210_FRAME_SIZE) {
    ESP_LOGV(TAG, "Frame too short: %zu bytes", length);
    return false;
  }
  
  ESP_LOGV(TAG, "Raw frame: %02X %02X %02X %02X %02X",
           data[0], data[1], data[2], data[3], data[4]);
  
  // 检查帧头
  if (data[0] != SY210_HEADER1 || data[1] != SY210_HEADER2) {
    ESP_LOGV(TAG, "Invalid header: %02X %02X (expected %02X %02X)", 
             data[0], data[1], SY210_HEADER1, SY210_HEADER2);
    return false;
  }
  
  // 提取数据值
  uint16_t pm_value = (static_cast<uint16_t>(data[2]) << 8) | data[3];
  
  // 计算校验和
  uint8_t calculated_checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  
  if (calculated_checksum != data[4]) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated=%02X, received=%02X", 
             calculated_checksum, data[4]);
    return false;
  }
  
  // 数据范围验证（PM值不应超过合理范围）
  if (pm_value > 9999) {  // 最大值检查
    ESP_LOGW(TAG, "PM value out of range: %d µg/m³", pm_value);
    return false;
  }
  
  ESP_LOGD(TAG, "Valid PM reading: %d µg/m³", pm_value);
  
  // 发布数据到相应的传感器
  // SY210通常只返回PM2.5数据，如果您的设备支持多种PM测量，
  // 需要根据具体协议调整这里的逻辑
  
  if (this->pm25_sensor_ != nullptr) {
    this->pm25_sensor_->publish_state(pm_value);
    ESP_LOGD(TAG, "Published PM2.5: %d µg/m³", pm_value);
  }
  
  // 如果您的SY210变种支持PM10和PM1.0，可以在这里添加逻辑
  // 根据不同的帧类型或字段来区分不同的PM值
  /*
  if (this->pm10_sensor_ != nullptr) {
    // 这里需要根据实际协议调整
    this->pm10_sensor_->publish_state(pm_value);
  }
  
  if (this->pm1_0_sensor_ != nullptr) {
    // 这里需要根据实际协议调整
    this->pm1_0_sensor_->publish_state(pm_value);
  }
  */
  
  return true;
}

}  // namespace sy210
}  // namespace esphome