-- Achievement Service
-- 负责加载策划配置并在运行时跟踪玩家的成就进度

local skynet = require "skynet"

local CMD = {}
local achievement_config = {}
local achievement_by_type = {}
local player_states = {}

local function build_type_index()
    achievement_by_type = {}
    for _, cfg in ipairs(achievement_config) do
        local bucket = achievement_by_type[cfg.type]
        if not bucket then
            bucket = {}
            achievement_by_type[cfg.type] = bucket
        end
        table.insert(bucket, cfg)
    end
end

local function load_config()
    local root = skynet.getenv("root") or "../../"
    local path = root .. "config/hearthstone/achievement.lua"
    local chunk, err = loadfile(path)
    if not chunk then
        error(string.format("[Achievement] Failed to load config '%s': %s", path, err))
    end

    local ok, data = pcall(chunk)
    if not ok then
        error(string.format("[Achievement] Error evaluating '%s': %s", path, data))
    end

    local list = data
    if type(list) == "table" and list.list then
        list = list.list
    end

    if type(list) ~= "table" then
        error(string.format("[Achievement] Invalid config structure in '%s'", path))
    end

    achievement_config = list
    build_type_index()
    skynet.error(string.format("[Achievement] Loaded %d achievement records", #achievement_config))
end

local function ensure_player_state(player_id)
    local state = player_states[player_id]
    if not state then
        state = {}
        player_states[player_id] = state
    end
    return state
end

local function record_completion(state, cfg, completed)
    local entry = state[cfg.id]
    if not entry then
        entry = { progress = 0, completed = false }
        state[cfg.id] = entry
    end

    if entry.completed then
        return
    end

    entry.progress = (entry.progress or 0)
    entry.progress = math.min(entry.progress, cfg.condition or entry.progress)
    entry.reward = entry.reward or cfg.reward

    table.insert(completed, { id = cfg.id, reward = cfg.reward })
    entry.completed = true
end

function CMD.update_progress(player_id, event_type, delta)
    delta = delta or 1
    local related = achievement_by_type[event_type]
    if not related then
        return { updated = false, completed = {} }
    end

    local state = ensure_player_state(player_id)
    local completed = {}

    for _, cfg in ipairs(related) do
        local entry = state[cfg.id]
        if not entry then
            entry = { progress = 0, completed = false }
            state[cfg.id] = entry
        end

        if not entry.completed then
            entry.progress = entry.progress + delta
            if entry.progress >= (cfg.condition or 0) then
                record_completion(state, cfg, completed)
            end
        end
    end

    if #completed > 0 then
        skynet.error(string.format("[Achievement] Player %s completed %d achievements", player_id, #completed))
    end

    return {
        updated = #completed > 0,
        completed = completed,
        state = state
    }
end

function CMD.get_player_state(player_id)
    return ensure_player_state(player_id)
end

function CMD.get_config()
    return achievement_config
end

function CMD.reload()
    load_config()
    return #achievement_config
end

function CMD.reset_player(player_id)
    player_states[player_id] = nil
end

skynet.start(function()
    load_config()

    skynet.dispatch("lua", function(session, source, cmd, ...)
        local f = CMD[cmd]
        if not f then
            skynet.error("[Achievement] Unknown command:", cmd)
            if session ~= 0 then
                skynet.ret(skynet.pack(nil, "unknown_command"))
            end
            return
        end

        if session == 0 then
            f(...)
        else
            skynet.ret(skynet.pack(f(...)))
        end
    end)

    skynet.error("[Achievement] Service started")
end)
