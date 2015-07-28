function uaddr_mk(secid, off)
    return secid * 0x10000000 + off
end

function ADDR(addr)
    if (addr >= 0x10000000) then
        return addr
    end
    if (addr >= 0x2C6770) then
        return uaddr_mk(4, addr - 0x2C6770)
    end
    if (addr >= 0x2AD000) then
        return uaddr_mk(3, addr - 0x2AD000)
    end
    if (addr >= 0x1DE000) then
        return uaddr_mk(1, addr - 0x1DE000)
    end
    if (addr >= 0x123000) then
        return uaddr_mk(2, addr - 0x123000)
    end
    return 0
end
