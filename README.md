# ESP32 电流监测与可视化系统

![项目展示图](https://github.com/user-attachments/assets/5f1cc5ce-07b0-42d6-99ac-52df20b25e20)

## 项目概述

本项目采用ESP32作为核心控制器，INA219作为电流采集模块，实现一种低成本、可部署、可联网、可远程监控的功耗监测方案。

## 系统架构
![image](https://github.com/user-attachments/assets/6faddd2f-a084-4eef-aba8-c526fadb376d)

### 硬件设计

#### 设备结构

- **ESP32模块**：负责数据处理、Wi-Fi连接与数据上报
- **INA219模块**：通过定制的干电池采电装置将INA219串联接入低功耗设备电路，负责采集被测设备的瞬时电流值，通过I2C接口与ESP32通信
- **Type-C电源输入**：为ESP32与INA219供电，系统整体功耗低，可使用移动电源或USB供电

### 软件实现

- ESP32内置程序每隔一秒读取INA219的瞬时电流值
- 读取到的数据通过HTTP请求上传至LeanCloud云数据库，每分钟一次
- 云端存储后，通过静态网页调用LeanCloud API，实时展示功耗变化折线图，提供简单的按设备筛选和按日期筛选功能

### 可视化界面

- 网页前端采用HTML+JavaScript，使用图表库（如Chart.js）实时绘图
- 支持多设备查询与历史数据分析

## 运行流程

上电 → 连接Wi-Fi → 定时采样电流值 → 上传云端 → 前端展示
