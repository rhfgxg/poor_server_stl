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

 Date: 06/02/2025 18:20:19
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for player_achievements_quests
-- ----------------------------
DROP TABLE IF EXISTS `player_achievements_quests`;
CREATE TABLE `player_achievements_quests`  (
  `player_id` int NOT NULL COMMENT '玩家ID',
  `achievements` json NOT NULL COMMENT '成就',
  `quests` json NOT NULL COMMENT '任务',
  PRIMARY KEY (`player_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '玩家任务和成就' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
