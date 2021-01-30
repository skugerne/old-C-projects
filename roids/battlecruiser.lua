print("Lua is loading!")

require "math"
require "string"

radius = 150;   -- how large the collision circle and shield are

mass = 100;   -- how heavy in collisions

-- each pre-scaled for DT of 1/1000
turnRate = .04;         -- how fast it turns .... 0.075 = 75 deg/S
enginePower = .000125;  -- how fast it accelerates .... 0.00125 = 1250 PPS^2
maxSpeed = 1.0;         -- some kind of max speed ... 1.0 = 1000 PPS

shieldPoints = 100;

weapons = {
  {
    x = 140,         -- position on the model
    y = 0,
    angle = 0,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 20,   -- compared to 1000 updates per second
    glowLimit = 30,
    glowCoolRate = 0.75,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_DOUBLE',
    shotmod = 'SHOT_FAST_LONG'
  },
  {
    x = 140,         -- position on the model
    y = -10,
    angle = 30,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 80,   -- compared to 1000 updates per second
    glowLimit = 20,
    glowCoolRate = 0.5,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_SINGLE',
    shotmod = 'SHOT_FAST_LONG'
  },
  {
    x = 140,         -- position on the model
    y = 10,
    angle = -30,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 80,   -- compared to 1000 updates per second
    glowLimit = 20,
    glowCoolRate = 0.5,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_SINGLE',
    shotmod = 'SHOT_FAST_LONG'
  }
}

engines = {
  {
    firingMod = 6,
    firingPoint = 2,
    x = -105,   -- position on the model
    y = 24
  },
  {
    firingMod = 6,
    firingPoint = 4,
    x = -105,
    y = -24
  },
  {
    firingMod = 6,
    firingPoint = 0,
    x = -105,
    y = 0
  }
}


function draw(timestamp)

  --front head SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(96,-30)
    glVertex2i(96,30)
    glVertex2i(120,18)
    glVertex2i(120,-18)
  glEnd()

  --middle head SKUG
  glColor3f(1,0,0)
  glBeginPolygon()
    glVertex2i(96,-30)
    glVertex2i(96,30)
    glVertex2i(72,30)
    glVertex2i(72,-30)
  glEnd()

  -- little flashing thing
  if math.floor(timestamp / 250) % 2 == 0 then
    glColor3f(0,0,1)
  else
    glColor3f(0,1,0)
  end
  glBeginPolygon()
    glVertex2i(85,-3)
    glVertex2i(85,3)
    glVertex2i(90,0)
  glEnd()

  --lower head SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(48,-18)
    glVertex2i(48,18)
    glVertex2i(72,30)
    glVertex2i(72,-30)
  glEnd()

    -- red neck SKUG
  glColor3f(1,0,0)
  glBeginPolygon()
    glVertex2i(48,-18)
    glVertex2i(48,18)
    glVertex2i(0,18)
    glVertex2i(0,-18)
  glEnd()

  -- white body front SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-30,36)
    glVertex2i(0,18)
    glVertex2i(0,-18)
  glEnd()

  --big bad red box SKUG
  glColor3f(1,0,0)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-30,36)
    glVertex2i(-96,36)
    glVertex2i(-96,-36)
  glEnd()

  --left wing SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,36)
    glVertex2i(-124,14*6)
    glVertex2i(-124,60)
    glVertex2i(-96,36)
  glEnd()

  --right wing SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-124,-14*6)
    glVertex2i(-124,-60)
    glVertex2i(-96,-36)
  glEnd()

  --middle motor
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,-4)
    glVertex2i(-96,4)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,6)
    glVertex2i(-102,-6)
  glEnd()

  --left motor
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,-28)
    glVertex2i(-96,-20)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,-18)
    glVertex2i(-102,-30)
  glEnd()

  --right motor
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,28)
    glVertex2i(-96,20)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,18)
    glVertex2i(-102,30)
  glEnd()

end


-- recursively dump a table
-- from https://stackoverflow.com/questions/9168058/how-to-dump-a-table-to-console
function dump(o)
  if type(o) == 'table' then
     local s = '{ '
     for k,v in pairs(o) do
        if type(k) ~= 'number' then k = '"'..k..'"' end
        s = s .. '['..k..'] = ' .. dump(v) .. ','
     end
     return s .. '} '
  else
     return tostring(o)
  end
end


-- keep angles between +PI and -PI
function boundAngle(a)
  if a > math.pi then
    a = a - 2 * math.pi
  elseif a < -math.pi then
    a = a + 2 * math.pi
  end
  return a
end


-- the squared distance between two objects
function sqDistance(ob1, ob2)
  return (ob1.x-ob2.x)^2 + (ob1.y-ob2.y)^2
end


-- how rapidly (scaled to DT) the given object is getting closer (pos) or farther away (neg)
function makeReport(ob1, ob2)

  -- find the bearing of the object
  -- should be bound between -PI and +PI
  -- if ob1 has lower X & Y than ob2, bearing is between 0 and 90 deg
  local distX = ob2.x - ob1.x
  local distY = ob2.y - ob1.y
  local bearing = math.atan(distY, distX)

  -- find the relative heading of the object
  -- should be bound between -PI and +PI
  -- if ob1 has lower X & Y than ob2 but is gaining in both, relative heading is between -90 and -180 deg
  -- if bearing and heading are pointed directly opposite, the objects are moving towards each other
  local movX = ob1.dx - ob2.dx
  local movY = ob1.dy - ob2.dy
  local relativeHeading = math.atan(movY, movX)

  return {
    sqDist = distX^2 + distY^2,
    bearing = bearing,
    sqMove = movX^2 + movY^2,
    relativeHeading = relativeHeading
  }
end


-- determine if an object with gravity is too close for comfort
function dangerouslyCloseMassive(myself, massiveOb)
  if massiveOb.g > 0 then
    local dd = sqDistance(myself, massiveOb)
    if dd <= 0 or massiveOb.g / dd > 0.00001 then
      return true
    end
  end
  return false
end


function avoid(myself, massiveOb)
  print("Dangerously close to massive object.")
  props = makeReport(myself, massiveOb)

  print(
    string.format(
      "AI update in Lua ... myself (%0.02f,%0.02f) ... star (%0.02f,%0.02f).",
      myself.x,
      myself.y,
      massiveOb.x,
      massiveOb.y
    )
  )

  print(
    string.format(
      "... facing (%0.02f) ... star relative heading (%0.02f) ... star bearing (%0.02f).",
      myself.angle,
      props.relativeHeading,
      props.bearing
    )
  )

  local absAngDiff = math.abs(boundAngle(props.bearing - props.relativeHeading))
  local targetAngleDiff = boundAngle(boundAngle(props.bearing - math.pi) - myself.angle)
  local absTargetAngleDiff = math.abs(targetAngleDiff)
  print(
    string.format(
      "... difference between bearing and relative heading of star is (abs val) (%0.02f).",
      absAngDiff
    )
  )
  print(
    string.format(
      "... difference between current facing and ideal facing is (abs val) (%0.02f).",
      absTargetAngleDiff
    )
  )

  if (absTargetAngleDiff < math.pi * 0.65) or (absAngDiff > math.pi * 0.25) then
    setEngine(true)
  else
    setEngine(false)
  end

  if (absTargetAngleDiff < math.pi * 0.20) or (absAngDiff > math.pi * 0.75) then
    print("No turning.")
    setTurnLeft(false)
    setTurnRight(false)
  else
    if targetAngleDiff < 0 then
      print("Turn right.")
      setTurnLeft(false)
      setTurnRight(true)
    else
      print("Turn left.")
      setTurnLeft(true)
      setTurnRight(false)
    end
  end
end


function aiUpdate(world)

  -- the C side keeps things in 0-2pi, Lua side is +pi to -pi
  world.myself.angle = boundAngle(world.myself.angle)

  print(string.format("Time is %d.",world.timestamp))

  if dangerouslyCloseMassive(world.myself, world.massiveObjects[1]) then
    -- eventually convert to a loop over all massive objects
    setAiFlee(true)
    setAiAttack(false)
    setAiSearch(false)
    avoid(world.myself, world.massiveObjects[1])
  else
    setAiFlee(false)
    setAiAttack(true)
    setAiSearch(true)
    setEngine(false)

    results = findHot(0.1,0.1)
    local targX
    local targY
    if type(results.objects[1]) ~= "nil" then
      targX = results.objects[1].x     -- hottest, not a function of distance
      targY = results.objects[1].y
    elseif type(results.sectors[1]) ~= "nil" then
      targX = results.sectors[1].x     -- hottest, which is also a function of distance
      targY = results.sectors[1].y
    else
      targX = world.maxCoordinate / 2
      targY = world.maxCoordinate / 2
    end

    local distX = targX - world.myself.x
    local distY = targY - world.myself.y
    local bearing = math.atan(distY, distX)
    local anglediff = boundAngle(world.myself.angle - bearing)
    local absAnglediff = math.abs(anglediff)
    if absAnglediff < math.pi * 0.05 then
      setEngine(true)
      setTurnLeft(false)
      setTurnRight(false)
    else
      if anglediff > 0 then
        print("Turn right.")
        setTurnLeft(false)
        setTurnRight(true)
      else
        print("Turn left.")
        setTurnLeft(true)
        setTurnRight(false)
      end
    end
  end
end
