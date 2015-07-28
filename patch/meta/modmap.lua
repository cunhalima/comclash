--MDIR = "patch/meta/"
--print("Hello")
local file = io.open(MDIR .. "modules.map", "r")
local state = 0
begin_transaction()
while true do
    line = file:read()
    if line == nil then break end
    if state == 0 then
        if string.find(line,"MEMORY MAP") then state = 1 end
    elseif state < 5 then
        state = state + 1
    elseif state == 5 then
        local skip = false
        if string.len(line) == 0 then skip = true end
        local c = string.sub(line, 1, 1)
        if c == '(' then skip = true end
        if c == '#' then skip = true end
        --[[
        if string.len(line) == 0 then break end
        local section = string.sub(line, 5, 5)
        local offset  = string.sub(line, 8, 15)
        local name    = string.sub(line, 22, -1)
        if name == "_edata" or name == "_end" or name == "start" then skip = true end
        if not skip then
            local address = tonumber(section .. offset, 16)
            --print(address, name)
            NA(address, name)
        end
        ]]--
        if not skip then
            local name = string.sub(line, 1, 20)
            name = name:gsub("%s", "")
            if string.sub(name, 1, 1) == '?' then
                name = string.sub(name, 2, -1)
            end
            local addr  = tonumber(string.sub(line, 21, 26), 16)
            --print("name='" .. name .. "' addr =", addr)
            --print(line)
            slice_create(name, ADDR(addr))
        end
    end
end
end_transaction()
file:close()
