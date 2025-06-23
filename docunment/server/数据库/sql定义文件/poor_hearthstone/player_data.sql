/*
 Navicat Premium Dump SQL

 Source Server         : 本地数据库
 Source Server Type    : MySQL
 Source Server Version : 80034 (8.0.34)
 Source Host           : localhost:3306
 Source Schema         : hearthstone

 Target Server Type    : MySQL
 Target Server Version : 80034 (8.0.34)
 File Encoding         : 65001

 Date: 06/02/2025 18:13:18
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for playerdata
-- ----------------------------
DROP TABLE IF EXISTS `player_data`;
CREATE TABLE `player_data`  (
  `player_id` int NOT NULL AUTO_INCREMENT COMMENT '玩家ID',
  `user_id` int NOT NULL COMMENT '用户ID',
  `nickname` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '昵称',
  `gold` int NULL DEFAULT 0 COMMENT '金币',
  `arcane_dust` int NULL DEFAULT 0 COMMENT '奥术之尘',
  `arena_tickets` int NULL DEFAULT 0 COMMENT '竞技场兑换券',
  PRIMARY KEY (`player_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 2 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '玩家基本信息' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
