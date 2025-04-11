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

 Date: 06/02/2025 18:17:48
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for player_collection
-- ----------------------------
DROP TABLE IF EXISTS `player_collection`;
CREATE TABLE `player_collection`  (
  `player_id` int NOT NULL COMMENT '玩家ID',
  `card_collection` json NOT NULL COMMENT '卡牌收藏',
  `skins` json NOT NULL COMMENT '皮肤',
  PRIMARY KEY (`player_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '玩家收藏和皮肤' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
