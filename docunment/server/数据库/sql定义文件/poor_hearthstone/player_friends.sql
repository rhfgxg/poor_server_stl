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

 Date: 06/02/2025 18:24:49
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for player_friends
-- ----------------------------
DROP TABLE IF EXISTS `player_friends`;
CREATE TABLE `player_friends`  (
  `player_friend_id` int NOT NULL AUTO_INCREMENT COMMENT '玩家好友ID',
  `player_id` int NOT NULL COMMENT '玩家ID',
  `friend_id` int NOT NULL COMMENT '好友ID',
  `friend_remark` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '好友备注',
  `added_date` datetime NOT NULL COMMENT '添加日期',
  PRIMARY KEY (`player_friend_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 2 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '玩家好友' ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
