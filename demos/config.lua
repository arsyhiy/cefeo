function on_key(ch)
    if ch == 27 then -- ESC
        os.exit()
    elseif ch == 115 then -- s
        local x,y = get_cursor()
        set_line(y, "Вы нажали s на строке "..y)
        return true
    end
    return false
end

