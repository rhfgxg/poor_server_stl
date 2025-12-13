local achievement_list = {
    {
        id = 1,
        name = "Spell Master",
        description = "Use 10 spells in a single match",
        type = "use_spells",
        condition = 10,
        reward = "100 gold"
    },
    {
        id = 2,
        name = "Card Collector",
        description = "Collect 100 cards",
        type = "collect_cards",
        condition = 100,
        reward = "1 rare card"
    },
    {
        id = 3,
        name = "Game Finisher",
        description = "Complete a match",
        type = "complete_match",
        condition = 1,
        reward = "50 gold"
    }
}

-- 返回浅拷贝，避免运行时意外修改原始表
local function clone(tbl)
    local copy = {}
    for i, value in ipairs(tbl) do
        if type(value) == "table" then
            local entry = {}
            for k, v in pairs(value) do
                entry[k] = v
            end
            copy[i] = entry
        else
            copy[i] = value
        end
    end
    return copy
end

return {
    list = clone(achievement_list)
}

