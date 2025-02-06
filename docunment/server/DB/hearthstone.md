# 炉石传说数据库（hearthstone）
## 卡牌数据，任务，成就等静态数据
保存在 lua 配置文件中，服务器启动时加载到内存中，不保存在数据库中

## 玩家数据
### 玩家基本信息表（player_data）
- 玩家id（player_id）
- 用户id（user_id）  // 外键，关联到用户表
- 昵称（nickname）
- 等级（level）
- 经验值（experience_points）
- 金币（gold）

### 玩家收藏和皮肤表（player_collection）
- 收藏id（collection_id）
- 玩家id（player_id）  // 外键，关联到玩家基本信息表
- 卡牌收藏（card_collection）  // json字符串，键值对形式保存卡牌id和数量
- 皮肤（skins）  // json字符串，存储多个皮肤id

卡牌收藏的 JSON 字符串格式如下：
```json
{
	"CardID1": 2,
	"CardID2": 1,
	"CardID3": 3
}
```
皮肤的 JSON 字符串格式如下：
```json
[id1, id2, id3]
```

### 玩家任务和成就表（player_achievements_quests）
- 任务成就id（achievement_quest_id）
- 玩家id（player_id）  // 外键，关联到玩家基本信息表
- 成就（achievements）  // json字符串，保存已完成的成就id列表
- 任务（quests）  // json字符串，保存当前任务的id和状态

成就的 JSON 字符串格式如下：
```json
{
	"成就id": 
	{
		"进度": 1,
		"成就状态": 未开启/进行中/已完成
	},
	"成就id": 
	{
		"进度": 1,
		"成就状态": 未开启/进行中/已完成
	},
}
```

### 玩家职业胜场表（player_class_wins）
- 职业胜场id（class_win_id）
- 玩家id（player_id）  // 外键，关联到玩家基本信息表
- 职业胜场（class_wins）  // json字符串，键值对形式保存职业id和胜场数
- 对战场次（matches_played）  // json字符串，键值对形式保存各模式的对战场次

职业胜场的 JSON 字符串格式如下：
```json
{
	"职业ID1": 2,
	"职业ID2": 1,
	"职业ID3": 3
}
```
对战场次的 JSON 字符串格式如下：
```json
{
	"模式1": 2,
	"模式2": 1,
	"模式3": 3
}
```

### 玩家好友表（player_friends）
- 玩家好友id（player_friend_id）
- 玩家id（player_id）  // 外键，关联到玩家基本信息表
- 好友id（friend_id）  // 外键，关联到玩家基本信息表
- 好友备注（friend_remark）
- 添加日期（added_date）
