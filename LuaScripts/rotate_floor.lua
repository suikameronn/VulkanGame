

function Guragura(rotateY,speed)

    while true do
        local finish = true

        if(math.abs(rotateY - Rotate.z) >= math.abs(speed)) then
            Rotate.z = Rotate.z + speed
            finish = false
        end

        if(finish) then
            break
        end

        coroutine.yield()
    end

    coroutine.yield()
end

function Reset()

    Rotate.z = 0

    coroutine.yield()
end

function Wait(waitFrame)

    for i = 0,waitFrame,1 do
        coroutine.yield()
    end
end

function Update()
    coroutine.yield()

    local speed = 2.0
    local speed2 = 4.0

    local waitFrame = 10

    --10度くらいに揺らす
    local y1 = 5
    --1回転させる
    local y2 = 180

    while true do

        for i = 0,10,1 do
            Guragura(y1,speed)--回転前に揺らす
            Guragura(-y1,-speed)--回転前に揺らす
        end
        Guragura(y2,speed2)--大きく回転させる
        Reset()--回転をリセットする
        Wait(waitFrame)--一定フレーム待つ
    end

end