function update()
    while true do
        coroutine.yield("a");
        coroutine.yield("b");
    end
end