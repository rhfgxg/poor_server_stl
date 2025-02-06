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

 Date: 06/02/2025 18:21:46
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for player_class_wins
-- ----------------------------
DROP TABLE IF EXISTS `player_class_wins`;
CREATE TABLE `player_class_wins`  (
  `player_id` int NOT NULL COMMENT '玩家ID',
  `class_wins` json NOT NULL COMMENT '职业胜场',
  `matches_played` json NOT NULL COMMENT '对战场次',
  PRIMARY KEY (`player_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '玩家职业胜场' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
