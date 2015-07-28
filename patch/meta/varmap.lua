--MDIR = "patch/meta/"
--print("Hello")
local file = io.open(MDIR .. "cc.map", "r")
local state = 0
begin_transaction()
while true do
    line = file:read()
    if line == nil then break end
    if state == 0 then
        if string.find(line,"Address") then state = 1 end
    elseif state == 1 then
        state = 2
    elseif state == 2 then
        if string.len(line) == 0 then break end
        local section = string.sub(line, 5, 5)
        local offset  = string.sub(line, 8, 15)
        local name    = string.sub(line, 22, -1)
        local skip = false
        if name == "_edata" or name == "_end" or name == "start" then skip = true end
        if not skip then
            local address = tonumber(section .. offset, 16)
            section = tonumber(section, 16)
            if section == 1 then
                --print("PUBLIC " .. name)
                NP(address, name)
            else
                --- ALL PUBLIC???
                NP(address, name)
            end
        end
    end
end
end_transaction()
file:close()
