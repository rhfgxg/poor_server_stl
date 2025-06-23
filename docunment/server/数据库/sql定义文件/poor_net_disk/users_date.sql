/*
 Navicat Premium Dump SQL

 Source Server         : 本地数据库
 Source Server Type    : MySQL
 Source Server Version : 80034 (8.0.34)
 Source Host           : localhost:3306
 Source Schema         : poor_file_hub

 Target Server Type    : MySQL
 Target Server Version : 80034 (8.0.34)
 File Encoding         : 65001

 Date: 11/04/2025 08:16:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for users_date
-- ----------------------------
DROP TABLE IF EXISTS `users_date`;
CREATE TABLE `users_date`  (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '作为网盘ID',
  `user_id` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '用户ID，复制于用户表',
  `storage_address` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '文件存储路径',
  `total_space` bigint NOT NULL DEFAULT 10737418240 COMMENT '用户总存储空间，单位为字节，默认10GB',
  `used_space` bigint NOT NULL DEFAULT 0 COMMENT '用户已使用的存储空间',
  `plan_type` enum('free','premium','enterprise') CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT 'free' COMMENT '用户套餐类型：free-免费，premium-付费，enterprise-企业',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `user_id`(`user_id` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '网盘用户信息表' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
