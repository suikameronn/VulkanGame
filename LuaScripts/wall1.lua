

function Move(destPos,speed)

    while true do
        local finish = true

        if(math.abs(destPos.x - Position.x) >= math.abs(speed)) then
            Position.x = Position.x + speed
            finish = false
        end

        if(math.abs(destPos.y - Position.y) >= math.abs(speed)) then
            Position.y = Position.y + speed
            finish = false
        end

        if(math.abs(destPos.z - Position.z) >= math.abs(speed)) then
            Position.z = Position.z + speed
            finish = false
        end

        if(finish) then
            break
        end

        coroutine.yield()
    end

    coroutine.yield()
end

function Update()
    coroutine.yield()
    
    local speed = 1.0

    local destPos1 = {x = Position.x + 50.0,y = Position.y,z = Position.z}
    local destPos2 = {x = Position.x - 50.0,y = Position.y,z = Position.z}

    while true do
        Move(destPos1,speed)
        Move(destPos2,-speed)
    end

    print("Finish")
end